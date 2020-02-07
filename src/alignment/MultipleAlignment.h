#ifndef MMSEQS_MULTIPLEALIGNMENT_H
#define MMSEQS_MULTIPLEALIGNMENT_H

#include "Matcher.h"
#include "Sequence.h"
#include "SubstitutionMatrix.h"

class MultipleAlignment {
public:
    enum alignment_element {
        ANY=20,   //number representing an X (any amino acid) internally
        NAA=20,   //number of amino acids (0-19)
        GAP=21,   //number representing a gap internally
        ENDGAP=22 //number representing a ignored gaps (for some calculations like gap percentage)
    };

    struct MSAResult {
        size_t msaSequenceLength;
        size_t centerLength;
        size_t setSize;
        char **msaSequence;
        char *keep;

        MSAResult(size_t msaSequenceLength, size_t centerLength, size_t setSize, char **msa)
                : msaSequenceLength(msaSequenceLength), centerLength(centerLength), setSize(setSize), msaSequence(msa), keep(NULL) {}
    };


    MultipleAlignment(size_t maxSeqLen, size_t maxSetSize, SubstitutionMatrix *subMat);

    ~MultipleAlignment();

    static void print(MSAResult msaResult, SubstitutionMatrix * subMat);

    // init aligned memory for the MSA
    static char *initX(int len);

    MSAResult computeMSA(Sequence *pSequence, const std::vector<Sequence *> &vector, const std::vector<Matcher::result_t> &vector1, bool i);

    // clean memory for MSA
    static void deleteMSA(MultipleAlignment::MSAResult * res);

private:
    BaseMatrix * subMat;

    size_t maxSeqLen;
    size_t maxSetSize;
    size_t maxMsaSeqLen;
    unsigned int * queryGaps;

    void computeQueryGaps(unsigned int *queryGaps, Sequence *center, const std::vector<Sequence *>& seqs, const std::vector<Matcher::result_t>& alignmentResults);

    size_t updateGapsInCenterSequence(char **msaSequence, Sequence *centerSeq, bool noDeletionMSA);

    void updateGapsInSequenceSet(char **centerSeqSize, size_t seqs, const std::vector<Sequence *> &vector,
                                                    const std::vector<Matcher::result_t> &queryGaps, unsigned int *noDeletionMSA,
                                                    bool b);

    MSAResult singleSequenceMSA(Sequence *centerSeq);

};

#endif //MMSEQS_MULTIPLEALIGNMENT_H
