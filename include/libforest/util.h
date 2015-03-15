#ifndef LIBF_UTIL_H
#define LIBF_UTIL_H

#include <vector>
#include <iostream>

#include "error_handling.h"
#include "fastlog.h"

/**
 * This is the buffer size for the arrays in the graph structures
 */
#define LIBF_GRAPH_BUFFER_SIZE 5000

/**
 * Quickly computes the entropy of a single bin of a histogram.
 */
#define ENTROPY(p) (-(p)*fastlog2(p))

namespace libf {
    // TODO: Remove this crap
    class Exception {
    public:
        Exception(const std::string & s) {}
    };
    
    /**
     * This class contains several use functions that are somewhat unrelated. 
     */
    class Util {
    public:
        /**
         * Creates a random permutation of [N]. 
         * TODO: Write unit test
         * 
         * @param N The number of points
         * @param sigma The permutation
         */
        static void generateRandomPermutation(int N, std::vector<int> & sigma);
        
        /**
         * Returns true if the given parameter is a valid permutation and returns
         * false otherwise. If sigma is of length N, then we check if sigma is in
         * S_N. The permutation must be given as a graph (n, sigma(n)). 
         * 
         * @param sigma The permutation. 
         * @return true if sigma is a valid permutation. 
         */
        static bool isValidPermutation(const std::vector<int> & sigma) throw()
        {
            // Initialize a list of flags to remember which images have several
            // points in the domain
            std::vector<bool> check(sigma.size(), false);
            
            for (size_t i = 0; i < sigma.size(); i++)
            {
                // If the given image is not in [N], then this cannot be a valid
                // permutation
                if (sigma[i] < 0 || sigma[i] >= static_cast<int>(sigma.size()))
                {
                    return false;
                }
                
                // Check if this image has already a partner
                if (check[sigma[i]])
                {
                    return false;
                }
                
                // Remember that we already used this image
                check[sigma[i]] = true;
            }
            
            return true;
        }
        
        /**
         * Applies a permutation to the vector in and saves it in the vector out. 
         * the permutation must be given as a mapping of the form n -> p(n). Thus
         * the n-th entry of the permutation vector must be the image under the
         * permutation. 
         * 
         * You must not set in = out. This would break the algorithm. 
         * 
         * The algorithm does not check if you pass a valid permutation. This
         * would create too much overhead.
         * 
         * @param permutation The permutation of the vector entries as a list of images
         * @param in The vector to permute
         * @param out The permuted vector
         */
        template <class T>
        static void permute(const std::vector<int> & permutation, const std::vector<T> & in, std::vector<T> & out) throw (AssertionException)
        {
            BOOST_ASSERT_MSG(permutation.size() == in.size(), "The permutation has invalid length.");
            BOOST_ASSERT_MSG(&in != &out, "The input vector must not be the same as the output vector.");
            BOOST_ASSERT_MSG(isValidPermutation(permutation), "The given vector does not encode a valid permutation.");

            // Make the output array of the correct size. 
            out.resize(in.size());

            // Copy the elements
            for (size_t i = 0; i < permutation.size(); i++)
            {
                out[permutation[i]] = in[i];
            }
        }
        
        /**
         * Computes the Hamming distance between two vectors of equal size. The
         * Hamming Distance is defined as the number of unequal entries of 
         * the two vectors. If the vectors are of unequal length, the distance
         * treats the "missing" entries as no match. 
         * 
         * @param v1 The first vector
         * @param v2 The second vector
         * @return The Hamming distance of v1 and v2
         */
        template <class T>
        static size_t hammingDist(const std::vector<T> & v1, const std::vector<T> & v2)
        {
            size_t result = 0;
            
            // Take care of vectors of unequal lengths
            if (v1.size() > v2.size())
            {
                result = v1.size() - v2.size();
            }
            else if (v2.size() > v1.size())
            {
                result = v2.size() - v1.size();
            }
            
            // Compute the actual distance
            for (size_t i = 0; i < std::min(v1.size(), v2.size()); i++)
            {
                if (v1[i] != v2[i])
                {
                    result++;
                }
            }
            
            return result;
        }
    
        /**
         * Dumps a vector to standard out. The elements of the vector must be
         * accepted by std::cout <<. This function is only for debug purposes. 
         * 
         * @param v The vector to dump
         */
        template <class T>
        void dumpVector(const std::vector<T> & v)
        {
            for (size_t i = 0; i < v.size(); i++)
            {
                std::cout << i << ": " << v[i] << std::endl;
            }

            std::cout.flush();
        }
        
        /**
         * Returns the index of an array. If there are multiple maxima in an 
         * array the smallest of those indices is returned. If the array is
         * empty, the result is 0. 
         * 
         * TODO: Write unit test
         * 
         * @param v The array of comparable objects
         * @return The maximizing index
         */
        template <class T>
        static size_t argMax(const std::vector<T> & v)
        {
            // If thee array is empty, there is not much to do. 
            if (v.size() == 0) return 0;
            
            // Determine the maximizing index
            T arg = v[0];
            size_t index = 0;
            for (size_t i = 1; i < v.size(); i++)
            {
                if (v[i] > arg)
                {
                    arg = v[i];
                    index = i;
                }
            }
            
            return index;
        }
    };

    /**
     * A histogram over the class labels. We use this for training.
     */
    class EfficientEntropyHistogram {
    public:
        /**
         * Default constructor. Initializes the histogram with 0 bins.
         */
        EfficientEntropyHistogram() : 
                bins(0),
                histogram(0),
                mass(0),
                entropies(0),
                totalEntropy(0) {}

        /**
         * Construct a entropy histogram of the given size. All bins are 
         * initialized with 0. 
         * 
         * @param bins The number of bins
         */
        EfficientEntropyHistogram(int bins) : 
            bins(0),
            histogram(0),
            mass(0),
            entropies(0),
            totalEntropy(0)
        {
            // Allocate the histogram
            resize(bins);
        }

        /**
         * Copy constructor.
         * 
         * @param other The histogram to copy
         */
        EfficientEntropyHistogram(const EfficientEntropyHistogram & other) : EfficientEntropyHistogram()
        {
            resize(other.bins);
            for (int i = 0; i < bins; i++)
            {
                histogram[i] = other.histogram[i];
                entropies[i] = other.entropies[i];
            }
            totalEntropy = other.totalEntropy;
            mass = other.mass;
        }

        /**
         * Assignment operator
         * 
         * @param other The object on the right side of the assignment operator
         */
        EfficientEntropyHistogram & operator= (const EfficientEntropyHistogram &other)
        {
            // Prevent self assignment
            if (this != &other)
            {
                resize(other.bins);
                for (int i = 0; i < bins; i++)
                {
                    histogram[i] = other.histogram[i];
                    entropies[i] = other.entropies[i];
                }
                totalEntropy = other.totalEntropy;
                mass = other.mass;
            }
            
            return *this;
        }

        /**
         * Destructor
         */
        ~EfficientEntropyHistogram()
        {
            // Only delete the arrays if they have been allocated
            if (histogram != 0)
            {
                delete[] histogram;
            }
            if (entropies != 0)
            {
                delete[] entropies;
            }
        }
        
        /**
         * Sets all entries in the histogram to 0. 
         */
        void reset()
        {
            for (int i = 0; i < bins; i++)
            {
                histogram[i] = 0;
                entropies[i] = 0;
            }
            totalEntropy = 0;
            mass = 0;
        }
        
        /**
         * Resizes the histogram to a certain size and initializes all bins
         * with 0 even if the size did not change. 
         * 
         * @param newBins The new number of bins
         */
        void resize(int newBins)
        {
            BOOST_ASSERT_MSG(newBins >= 0, "Bin count must be non-negative.");
            
            // Release the current histogram
            if (newBins != bins)
            {
                if (histogram != 0)
                {
                    delete[] histogram;
                    histogram = 0;
                }
                if (entropies != 0)
                {
                    delete[] entropies;
                    entropies = 0;
                }
            }

            bins = newBins;
            
            // Only allocate a new histogram, if there is more than one class
            if (newBins > 0)
            {
                histogram = new int[bins];
                entropies = new float[bins];
                
                reset();
            }
        }

        /**
         * Returns the size of the histogram (= class count)
         * 
         * @return The number of bins of the histogram
         */
        int getSize() const
        {
            return bins; 
        }

        /**
         * Get the histogram value for class i.
         * 
         * @return The value in bin i.
         */
        int at(const int i) const
        {
            BOOST_ASSERT_MSG(i >= 0 && i < bins, "Bin index out of range.");
            return histogram[i];
        }
        
        /**
         * Adds one instance of class i while updating entropy information.
         * 
         * @param i The bin to which a single point shall be added.
         */
        void addOne(const int i)
        {
            BOOST_ASSERT_MSG(i >= 0 && i < bins, "Invalid bin bin index.");

            totalEntropy += ENTROPY(mass);
            mass += 1;
            totalEntropy -= ENTROPY(mass);
            histogram[i]++;
            totalEntropy -= entropies[i];
            entropies[i] = ENTROPY(histogram[i]); 
            totalEntropy += entropies[i];
        }
        
        /**
         * Remove one instance of class i while updating the entropy information.
         * 
         * @param i The bin from which a single point shall be removed.
         */
        void subOne(const int i)
        {
            BOOST_ASSERT_MSG(i >= 0 && i < bins, "Invalid bin bin index.");
            BOOST_ASSERT_MSG(at(i) > 0, "Bin is already empty.");

            totalEntropy += ENTROPY(mass);
            mass -= 1;
            totalEntropy -= ENTROPY(mass);

            histogram[i]--;
            totalEntropy -= entropies[i];
            if (histogram[i] < 1)
            {
                entropies[i] = 0;
            }
            else
            {
                entropies[i] = ENTROPY(histogram[i]); 
                totalEntropy += entropies[i];
            }
        }
        
        /**
         * Returns the total mass of the histogram.
         * 
         * @return The total mass of the histogram
         */
        float getMass() const
        {
            return mass;
        }

        /**
         * Calculates the entropy of the histogram. This only works if the 
         * function initEntropies has been called before. 
         * 
         * @return The calculated entropy
         */
        float getEntropy() const
        {
            return totalEntropy;
        }

        /**
         * Returns true if the histogram has at most a single non-empty bin. 
         * 
         * @return true if the histogram is pure. 
         */
        bool isPure() const
        {
            bool nonPure = false;
            for (int i = 0; i < bins; i++)
            {
                if (histogram[i] > 0)
                {
                    if (nonPure)
                    {
                        return false;
                    }
                    else
                    {
                        nonPure = true; 
                    }
                }
            }
            
            return true;
        }

    private:
        /**
         * The number of classes in this histogram
         */
        int bins;

        /**
         * The actual histogram
         */
        int* histogram;

        /**
         * The integral over the entire histogram
         */
        float mass;

        /**
         * The entropies for the single bins
         */
        float* entropies;

        /**
         * The total entropy
         */
        float totalEntropy;
    };
}
#endif
 