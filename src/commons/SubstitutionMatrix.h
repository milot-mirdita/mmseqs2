#ifndef SUBSTITUTION_MATRIX_H
#define SUBSTITUTION_MATRIX_H

// Written by Maria Hauser mhauser@genzentrum.lmu.de
//  
// Represents a simple amino acid substitution matrix. 
// Can read a straight-forward substitution matrix file (.mat ending of the file) and probabilities file (.out ending of the file).
// If a probabilities file is given, it calculates a biased matrix (produces shorter = more precise alignments).
//

#include <cstddef>
#include "BaseMatrix.h"
#include "ProfileStates.h"

class SubstitutionMatrix : public BaseMatrix {
public:
    SubstitutionMatrix(const char *filename, float bitFactor, float scoreBias);

    virtual ~SubstitutionMatrix();

    virtual float getBitFactor() const {return bitFactor; }

    virtual double getBackgroundProb(size_t aa_index) { return pBack[aa_index]; }

    static void calcLocalAaBiasCorrection(const BaseMatrix *m ,const unsigned char *int_sequence, const int N, float *compositionBias, float scale);

    // radius d of the local window used by the compositional bias corrections
    // window around position i is [i-d, i+d] and 2*d+1 letters wide
    static const int COMP_BIAS_WINDOW_RADIUS = 20;

    /*
     * Query-side compositional bias correction by local arithmetic averaging of
     * letter probabilities.
     *
     * The uncorrected profile score of query position i against target letter b is
     *     S(i,b)  = log2( p_i(b) / f_db(b) )
     * with p_i(b) = p(b|x_i) taken from the substitution matrix. The correction
     * replaces the database background f_db by a local background
     *     f_i(b) = wLocal * 1/(2d+1) * sum_{k=i-d}^{i+d} p_k(b) + (1 - wLocal) * f_db(b)
     *     S'(i,b) = log2( p_i(b) / f_i(b) )
     * so the additive correction on top of the plain substitution matrix score is
     *     delta(i,b) = -log2( f_i(b) / f_db(b) ),
     * which depends on the target letter b but not on the query letter at i.
     *
     * compositionBias must hold N * m->alphabetSize floats and is filled in
     * position-major order, i.e. compositionBias[i * alphabetSize + b].
     */
    // compositionBiasDiagonal, if given, receives N floats: the correction projected onto the
    // query's own letter, for the stages that cannot score against a known target letter
    // (the k-mer threshold). Position i is left out of its own local background there, see below.
    static void calcLocalAaBiasCorrectionProfile(const BaseMatrix *m, const unsigned char *int_sequence,
                                                 const int N, float *compositionBias, float scale, float wLocal,
                                                 bool centerExpectation = false,
                                                 float *compositionBiasDiagonal = NULL);

    static void calcProfileProfileLocalAaBiasCorrection(short *profileScores,
                                                        const size_t profileAASize,
                                                        const int N,
                                                        size_t alphabetSize);
    static void calcProfileProfileLocalAaBiasCorrectionAln(int8_t *profileScores,
                                                           unsigned int N,
                                                           size_t alphabetSize,
                                                           BaseMatrix *subMat);
    static void calcGlobalAaBiasCorrection(const BaseMatrix * m,
                                           char *profileScores,
                                           float *pNullBuffer,
                                           const size_t profileAASize,
                                           const int N);
    bool estimateLambdaAndBackground(const double ** mat, int alphabetSize, double * pBack, double & lambda);

    bool mappingHasAminoAcidLetters();

    void setupLetterMapping();

    struct FastMatrix{
        const char ** matrix;
        const char * matrixData;
        const size_t asciiStart;
        FastMatrix(const char ** matrix, const char * matrixData, const size_t asciiStart):
                matrix(matrix), matrixData(matrixData), asciiStart(asciiStart)
        {}
    };

    // build matrix from ~ (=0) to ~(=122)
    static FastMatrix createAsciiSubMat(BaseMatrix & submat){
        const size_t asciiStart = 0;
        const size_t asciiEnd = 'z'+1;
        const size_t range = asciiEnd-asciiStart;
        char ** matrix = new char *[range];
        char * matrixData = new char[range*range];
        for(size_t i = 0; i < range; i++) {
            matrix[i] = matrixData+(i*range);
            int curr_i = static_cast<int>(submat.aa2num[asciiStart+i]);
            for (size_t j = 0; j < range; j++) {
                int curr_j = static_cast<int>(submat.aa2num[asciiStart+j]);
                matrix[i][j] = static_cast<char>(submat.subMatrix[curr_i][curr_j]);
            }
        }
        return FastMatrix((const char**) matrix,
                          (const char*) matrixData,
                          asciiStart);
    }

    // hack to support calculatelambda
    static bool printLambdaAndBackground;
private:
    int parseAlphabet(char * word, char * num2aa, int * aa2num);

    void readProbMatrix(const std::string &matrixData, bool containsX);

    std::pair<int, bool>  setAaMappingDetectAlphSize(std::string &matrixData);

    const float bitFactor;
};

#endif
