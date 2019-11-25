/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/pcclustering/processors/dataframetopcprawdata.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/indexmapper.h>

#include <memory>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameToPCPRawData::processorInfo_{
    "org.inviwo.DataFrameToPCPRawData",  // Class identifier
    "Data Frame To PCPRaw Data",         // Display name
    "Undefined",                         // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};
const ProcessorInfo DataFrameToPCPRawData::getProcessorInfo() const { return processorInfo_; }

DataFrameToPCPRawData::DataFrameToPCPRawData()
    : Processor()
    , dataframe_("dataframe")
    , pcpRawData_("pcpRawData")
    , columns_("columns", "Columns") {

    addPort(dataframe_);
    addPort(pcpRawData_);
    addProperty(columns_);

    dataframe_.onChange([&] {
        if (auto df = dataframe_.getData()) {
            for (auto prop : columns_.getProperties()) {
                prop->setVisible(false);
            }

            for (size_t i = 0; i < df->getNumberOfColumns(); i++) {
                auto col = df->getColumn(i);
                auto name = col->getHeader();
                auto id = util::stripIdentifier(name);
                auto prop = columns_.getPropertyByIdentifier(id);
                BoolProperty* boolProp = nullptr;
                if (!prop) {
                    auto newProp = std::make_unique<BoolProperty>(id, name, true);
                    newProp->setSerializationMode(PropertySerializationMode::All);
                    boolProp = newProp.get();
                    columns_.addProperty(newProp.release());
                } else {
                    IVW_ASSERT(dynamic_cast<BoolProperty*>(prop) != nullptr,
                               "Found property is not a bool property");
                    boolProp = static_cast<BoolProperty*>(prop);
                }
                boolProp->setVisible(true);
                boolProp->setMetaData<SizeMetaData>("index", i);
            }
        }
    });
}

namespace {
bool verifyEqualSize(std::vector<std::shared_ptr<const BufferBase>>& buffers) {
    if (buffers.empty()) {
        return true;
    }
    auto size = buffers.front()->getSize();
    for (auto buf : buffers) {
        if (buf->getSize() != size) {
            return false;
        }
    }
    return true;
}
}  // namespace

void DataFrameToPCPRawData::process() {
    auto df = dataframe_.getData();

    std::vector<std::shared_ptr<const BufferBase>> enabledBuffers;

    for (auto boolProp : columns_.getPropertiesByType<BoolProperty>()) {
        if (boolProp->getVisible() && boolProp->get()) {
            IVW_ASSERT(boolProp->hasMetaData<SizeMetaData>("index"),
                       "Property does not have metadata index");
            auto index = boolProp->getMetaData<SizeMetaData>("index")->get();
            enabledBuffers.push_back(df->getColumn(index)->getBuffer());
        }
    }
    IVW_ASSERT(verifyEqualSize(enabledBuffers), "Enabled buffers does not have same size");

    if (enabledBuffers.empty()) {
        throw Exception("No columns enabled", IVW_CONTEXT);
    }

    auto data = std::make_shared<ParallelCoordinatesPlotRawData>();
    auto rows = enabledBuffers.front()->getSize();
    data->data.reserve(rows * enabledBuffers.size());
    data->data.resize(rows * enabledBuffers.size());

    util::IndexMapper2D indexA(size2_t{rows, enabledBuffers.size()});
    util::IndexMapper2D indexB(size2_t{enabledBuffers.size(), rows});

    size_t i = 0;
    for (auto buf : enabledBuffers) {
        const auto x = i++;

        buf->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
            [&]<typename T, BufferTarget I>(const BufferRAMPrecision<T, I>* ram)->void {
                const std::vector<T>& vec = ram->getDataContainer();
                const auto [minIt, maxIt] = std::minmax_element(vec.begin(), vec.end());
                const float min = static_cast<const float>(*minIt);
                const float max = static_cast<const float>(*maxIt);

                data->minMax.emplace_back(min, max);
                size_t j = 0;
                for (const auto& v : vec) {
                    const auto y = j++;
                    const float value = (static_cast<float>(v) - min) / (max - min);
                    data->data[indexB(x, y)] = (value - 0.5f) * 2.f;
                }

                /*std::transform(vec.begin(), vec.end(), std::back_inserter(data->data),
                               [&](auto value) -> float {
                                   return (static_cast<float>(value) - min) / (max - min);
                               });*/
            });
    }

    pcpRawData_.setData(data);
}

}  // namespace inviwo
