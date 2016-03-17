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

#include <algorithm>
#include <assert.h>
#include <math.h>
#include <vector>

int numberOfPermutations(int n) {
    assert((n >= 0) && "Only positive numbers are allowed");
    // Cast to int will round towards zero
    return static_cast<int>((n + 1) / 2);
}


using Permutation = std::vector<int>;
std::vector<Permutation> getPermutations(const std::vector<int>& data) {
    assert((!data.empty()) && "Data must not be empty");
    const int nPermutations = numberOfPermutations(static_cast<int>(data.size()));
    const int n = static_cast<int>(data.size());

    auto mod = [](int x, int y) {
        // Implementation of Edward J. Wegman's definition of the modulo function
        if (x == 0 || x == y)
            return y;
        else if (x <= 0)
            return x + y;
        else
            return x % y;
    };

    // Initialize the result vector with 0s
    std::vector<Permutation> result(nPermutations, Permutation(data.size(), 0));

    // Generate the initial permutation
    Permutation& p = result[0];

    // Implementation of:
    // n_{k+1} = [n_k + (-1)^{k+1} * k] mod n
    // Formula A.1 in Edward J. Wegman, Hyperdimensional Data Analysis Using Parallel
    // Coordinates, Journal of the American Statistical Association, 1990
    p[0] = 1;
    for (int k = 1; k <= n - 1; ++k) {

        int v = static_cast<int>(p[k - 1] + pow(-1, k + 1) * k);
        p[k] = mod(v, n);
    }

    // Implementation of:
    // n_k^{j+1} = (n_k^j + 1) mod n
    // Formula A.2 in Edward J. Wegman, Hyperdimensional Data Analysis Using Parallel
    // Coordinates, Journal of the American Statistical Association, 1990
    for (int i = 1; i < nPermutations; ++i) {

        Permutation& current = result[i];
        const Permutation& previous = result[i - 1];

        for (int k = 0; k < n; ++k)
            current[k] = mod(previous[k] + 1, n);
    }


    return result;
}
