#include <modules/pcclustering/processors/pcpgui.h>

#include <modules/pcclustering/misc/cpr.h>
#include <modules/pcclustering/misc/parallelcoordinates_axis_permutation.h>

namespace inviwo {

const ProcessorInfo PCPGui::processorInfo_{
    "pers.bock.PCPGUI",  // Class identifier
    "PCP GUI",            // Display name
    "GUI",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPGui::getProcessorInfo() const {
    return processorInfo_;
}

PCPGui::PCPGui()
    : Processor()
    , _inport("pcp.in")
    , _nDimensions("_nDimensions", "Number of Dimensions", 0, 0, 32)
    , _nSubcluster("_nSubcluster", "Dimensionality of subclusters", 0, 0, 100)
    , _coloringDimension("_coloringDimension", "Coloring Dimension", 0, 0, 100)
    , _enabledDimensionsOptions("_enabledDimensionsOptions", "Enabled Dimensions")
    , _enabledDimensionsString("_enabledDimensionsString", "Enabled Dimensions")
    , _dimensionOrderingOptions("_dimensionOrderingOptions", "Dimension Ordering")
    , _dimensionOrderingString("_dimensionOrderingString", "Dimension Ordering")
{
    addPort(_inport);

    addProperty(_nDimensions);
    _nDimensions.setReadOnly(true);
    _nDimensions.setVisible(false);

    addProperty(_nSubcluster);
    addProperty(_enabledDimensionsOptions);
    addProperty(_enabledDimensionsString);
    _enabledDimensionsString.setReadOnly(true);
    _enabledDimensionsString.setVisible(false);

    addProperty(_coloringDimension);

    addProperty(_dimensionOrderingOptions);
    addProperty(_dimensionOrderingString);
    _dimensionOrderingString.setReadOnly(true);
    _dimensionOrderingString.setVisible(false);

    _enabledDimensionsOptions.onChange([this]() {
        if (_isUpdating)
            return;

        std::string id = _enabledDimensionsOptions.getSelectedDisplayName();

        std::stringstream s(id);

        std::istream_iterator<std::string> it(s);
        std::istream_iterator<std::string> end;
        std::vector<std::string> tokens(it, end);

        std::string result;
        for (int i = 0; i < _nDimensions; ++i) {
            std::string is = std::to_string(i);

            auto it = std::find(tokens.begin(), tokens.end(), is);

            if (it == tokens.end())
                result.push_back('0');
            else
                result.push_back('1');
        }

        std::reverse(result.begin(), result.end());
        _enabledDimensionsString = result;

    });
    _dimensionOrderingOptions.onChange([this]() {
        if (_isUpdating)
            return;
        if (_dimensionOrderingOptions.size() > 0) {
            std::string s = _dimensionOrderingOptions.getSelectedIdentifier();
            _dimensionOrderingString = s;
        }
        else
            _dimensionOrderingString = "";
    });

    _nSubcluster.onChange([this]() {
        auto to_string = [](const std::vector<int>& v)-> std::string {
            std::string res = "";
            for (int i : v)
                res += std::to_string(i) + " ";
            return res;
        };

        _isUpdating = true;

        std::vector<int> sequence(_nDimensions);
        std::iota(sequence.begin(), sequence.end(), 0);
        {
            // Enabled Dimensions
            _enabledDimensionsOptions.clearOptions();

            std::vector<std::vector<int>> permutations = cPr(sequence, _nSubcluster);


            for (int i = 0; i < permutations.size(); ++i) {
                const std::vector<int>& v = permutations[i];
                std::string resName = to_string(v);
                std::string identifier = resName;
                identifier.erase(
                    std::remove_if(identifier.begin(), identifier.end(), ::isspace),
                    identifier.end()
                    );
                _enabledDimensionsOptions.addOption(identifier, resName, i);
            }
        }

        {
            // Dimensions Ordering
            _dimensionOrderingOptions.clearOptions();

            if (_nSubcluster > 0) {
                std::vector<Permutation> permutations = getPermutations(sequence);

                for (Permutation& p : permutations) {
                    for (int& i : p)
                        --i;
                }

                std::string s = to_string(sequence);
                std::string t = s;
                t.erase(
                    std::remove_if(t.begin(), t.end(), ::isspace),
                    t.end()
                );

                _dimensionOrderingOptions.addOption(t, s, 0);
                for (int i = 0; i < permutations.size(); ++i) {
                    const std::vector<int>& v = permutations[i];
                    std::string resName = to_string(v);
                    std::string identifier = resName;
                    identifier.erase(
                        std::remove_if(identifier.begin(), identifier.end(), ::isspace),
                        identifier.end()
                    );
                    _dimensionOrderingOptions.addOption(identifier, resName, i+1);
                }
            }
        }

        _isUpdating = false;
        _enabledDimensionsOptions.propertyModified();
        _dimensionOrderingOptions.propertyModified();
    });

    _nDimensions.onChange([this]() {
        _nSubcluster.setMaxValue(_nDimensions);
        _nSubcluster = _nDimensions.get();
        _coloringDimension.setMaxValue(_nDimensions - 1);
    });
}

void PCPGui::process() {
    if (!_inport.hasData())
        return;

    _nDimensions = _inport.getData()->nDimensions;
}

}  // namespace
