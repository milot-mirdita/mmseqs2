template <typename F>
inline F simd_hmax(const F * in, unsigned int n) {
    F current = std::numeric_limits<F>::min();
    do {
        current = std::max(current, *in++);
    } while(--n);

    return current;
}

class Impl final : public UngappedAligner::Impl {
public:
    Impl(size_t maxSequenceLength,
         int aaSize,
         bool aaBiasCorrection,
         float aaBiasCorrectionScale)
        : maxSequenceLength(maxSequenceLength + 1),
          aaSize(aaSize),
          aaBiasCorrection(aaBiasCorrection),
          aaBiasCorrectionScale(aaBiasCorrectionScale),
          queryLength(0),
          alphabetSize(0),
          maxSegLen(static_cast<int32_t>((this->maxSequenceLength + (VECSIZE_INT * 4) - 1) / (VECSIZE_INT * 4))),
          bias(0),
          profileByte(nullptr),
          vHStore(nullptr),
          vHLoad(nullptr),
          querySequence(nullptr),
          compositionBias(nullptr),
          tmpCompositionBias(nullptr),
          queryMat(nullptr) {
        const size_t profileByteSize = static_cast<size_t>(this->aaSize) * maxSegLen * (VECSIZE_INT * 4);
        const size_t stateSize = static_cast<size_t>(maxSegLen) * sizeof(simd_int);
        profileByte = static_cast<int8_t*>(mem_align(ALIGN_INT, profileByteSize));
        vHStore = (simd_int*)mem_align(ALIGN_INT, stateSize);
        vHLoad = (simd_int*)mem_align(ALIGN_INT, stateSize);

        querySequence = new int8_t[this->maxSequenceLength];
        compositionBias = new int8_t[this->maxSequenceLength];
        tmpCompositionBias = new float[this->maxSequenceLength];
        queryMat = new int8_t[std::max(this->maxSequenceLength, static_cast<size_t>(this->aaSize)) * this->aaSize * 2];

        std::memset(querySequence, 0, this->maxSequenceLength * sizeof(int8_t));
        std::memset(compositionBias, 0, this->maxSequenceLength * sizeof(int8_t));
    }

    ~Impl() override {
        free(profileByte);
        free(vHStore);
        free(vHLoad);
        delete[] querySequence;
        delete[] compositionBias;
        delete[] tmpCompositionBias;
        delete[] queryMat;
    }

    void initQuery(const Sequence *q, const int8_t *mat, const BaseMatrix *m) override {
        const bool isProfile = Parameters::isEqualDbtype(q->getSequenceType(), Parameters::DBTYPE_HMM_PROFILE);
        queryLength = q->L;
        alphabetSize = m->alphabetSize;

        std::memcpy(querySequence, q->numSequence, q->L);

        int32_t compositionBiasMin = 0;
        if (!isProfile && aaBiasCorrection) {
            SubstitutionMatrix::calcLocalAaBiasCorrection(
                m,
                q->numSequence,
                q->L,
                tmpCompositionBias,
                aaBiasCorrectionScale
            );
            for (int i = 0; i < q->L; i++) {
                compositionBias[i] = static_cast<int8_t>(
                    (tmpCompositionBias[i] < 0.0) ? (tmpCompositionBias[i] - 0.5f) : (tmpCompositionBias[i] + 0.5f)
                );
                compositionBiasMin = std::min(compositionBiasMin, static_cast<int32_t>(compositionBias[i]));
            }
        } else {
            std::memset(compositionBias, 0, q->L * sizeof(int8_t));
        }

        if (isProfile) {
            std::memcpy(queryMat, mat, q->L * Sequence::PROFILE_AA_SIZE * sizeof(int8_t));
            std::memset(queryMat + ((alphabetSize - 1) * q->L), 0, q->L * sizeof(int8_t));
        } else {
            std::memcpy(queryMat, mat, alphabetSize * alphabetSize * sizeof(int8_t));
        }

        int32_t profileBias = 0;
        const int32_t matSize = isProfile ? (q->L * Sequence::PROFILE_AA_SIZE) : (alphabetSize * alphabetSize);
        for (int32_t i = 0; i < matSize; i++) {
            profileBias = std::min(profileBias, static_cast<int32_t>(mat[i]));
        }
        bias = static_cast<uint8_t>(std::abs(profileBias) + std::abs(compositionBiasMin));

        if (isProfile) {
            createQueryProfile<int8_t, VECSIZE_INT * 4, SmithWaterman::PROFILE>(
                profileByte, querySequence, nullptr, queryMat, q->L, alphabetSize, bias, 0, q->L);
        } else {
            createQueryProfile<int8_t, VECSIZE_INT * 4, SmithWaterman::SUBSTITUTIONMATRIX>(
                profileByte, querySequence, compositionBias, queryMat, q->L, alphabetSize, bias, 0, 0);
        }
    }

    int score(const unsigned char *db_sequence, int32_t db_length) const override {
        int i; // position in query bands (0,..,W-1)
        int j; // position in db sequence (0,..,dbseq_length-1)
        int element_count = (VECSIZE_INT * 4);
        // width of bands in query and score matrix = hochgerundetes LQ/16
        const int W = (queryLength + (element_count - 1)) / element_count;

        simd_int S; // 16 unsigned bytes holding S(b*W+i,j) (b=0,..,15)
        simd_int Smax = simdi_setzero();
        simd_int Soffset; // all scores in query profile are shifted up by Soffset to obtain pos values
        simd_int *s_prev, *s_curr; // pointers to Score(i-1,j-1) and Score(i,j), resp.
        simd_int *qji; // query profile score in row j (for residue x_j)
        simd_int *s_prev_it, *s_curr_it;
        simd_int *query_profile_it = (simd_int *) profileByte;

        // Load the score offset to all 16 unsigned byte elements of Soffset
        Soffset = simdi8_set(bias);
        s_curr = vHStore;
        s_prev = vHLoad;

        memset(vHStore, 0, W * sizeof(simd_int));
        memset(vHLoad, 0, W * sizeof(simd_int));

        // loop over db sequence positions
        for (j = 0; j < db_length; ++j) {
            // Get address of query scores for row j
            qji = query_profile_it + db_sequence[j] * W;

            // Load the next S value
            S = simdi_load(s_curr + W - 1);
            S = simdi8_shiftl(S , 1);

            std::swap(s_prev, s_curr);
            s_curr_it = s_curr;
            s_prev_it = s_prev;

            // loop over query band positions
            for (i = 0; i < W; ++i) {
                // Saturated addition and subtraction to score S(i,j)
                // S(i,j) = S(i-1,j-1) + (q(i,x_j) + Soffset)
                S = simdui8_adds(S, *(qji++));
                // S(i,j) = max(0, S(i,j) - Soffset)
                S = simdui8_subs(S, Soffset);
                // store S to s_curr[i]
                simdi_store(s_curr_it++, S);
                // Smax(i,j) = max(Smax(i,j), S(i,j))
                Smax = simdui8_max(Smax, S);

                // Load the next S and Smax values
                S = simdi_load(s_prev_it++);
            }
        }

        return simd_hmax((unsigned char *) &Smax, element_count);
    }

private:
    size_t maxSequenceLength;
    int aaSize;
    bool aaBiasCorrection;
    float aaBiasCorrectionScale;

    int32_t queryLength;
    int32_t alphabetSize;
    int32_t maxSegLen;
    uint8_t bias;

    int8_t *profileByte;
    simd_int *vHStore;
    simd_int *vHLoad;

    int8_t *querySequence;
    int8_t *compositionBias;
    float *tmpCompositionBias;
    int8_t *queryMat;
};

UngappedAligner::Impl *createImpl(size_t maxSequenceLength,
                                  int aaSize,
                                  bool aaBiasCorrection,
                                  float aaBiasCorrectionScale) {
    return new Impl(maxSequenceLength, aaSize, aaBiasCorrection, aaBiasCorrectionScale);
}
