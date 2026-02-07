#include "simd.h"
#include "UngappedAligner.h"

#include "StripedSmithWaterman.h"
#include "Parameters.h"
#include "Sequence.h"
#include "SubstitutionMatrix.h"
#include "Util.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace UngappedDefault {

#include "UngappedAlignerInternal.cpp"

} // namespace UngappedDefault
