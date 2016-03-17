/*
Copyright 2016 Alexander Bock. All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or other
materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either
expressed or implied, of the FreeBSD Project.
*/

constexpr int factorial(int n) {
    return n > 0 ? n * factorial(n - 1) : 1;
}

template <typename T>
void cPrImpl(
    const std::vector<T>& set,  // The set for which to create the permutation
    size_t pos,                 // The current location in the set
    std::vector<std::vector<T>>& result,    // The resulting list of all permutations
    size_t bufferPos,           // The current location in the buffer list
    std::vector<T>& buffer)     // Buffer for the currently generated result
{
    if (bufferPos == buffer.size()) {
        // If we reach the end of the buffer, we have a permutation
        result.push_back(buffer);
        return;
    }

    if (pos == set.size()) {
        // If we reach the end of the set without having reached the end of the buffer
        // We do not have a permutation
        return;
    }

    // Add the current item to the result buffer
    buffer[bufferPos] = set[pos];
    // Recurse with the current item
    cPrImpl(set, pos + 1, result, bufferPos + 1, buffer);
    // Recurse without the current item (by overwriting it in the call)
    cPrImpl(set, pos + 1, result, bufferPos, buffer);
}

template <typename T>
std::vector<std::vector<T>> cPr(
    std::vector<T> set,         // The set for which to create the permutation
    int n)                      // The length of each permutation
{
    assert((n >= 0) && "Only positive permutation lenghts are premitted");
    assert((!set.empty()) && "Set must not be empty");
    assert(
        (static_cast<int>(n) <= set.size()) &&
        "Length of permutation must be smaller than the set"
        );

    // The buffer in which to cumulatively store the result while recursing
    std::vector<T> buffer(n);
    // The resulting buffer containing all unordered permutations
    std::vector<std::vector<T>> result;

    // Compute the number of permutations according to
    // http://mathworld.wolfram.com/Permutation.html
    // nCr = n! / r!(n-r)!
    const int nPermutations = factorial(set.size()) / factorial(n) * factorial(set.size() - n);
    // Preallocate that amount of memory for efficiency
    result.reserve(nPermutations);

    // The call starting the recursion
    cPrImpl(set, 0, result, 0, buffer);
    return result;
}