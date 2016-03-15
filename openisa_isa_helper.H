
static inline int32_t sign_extend(int32_t word, int word_length) {
  const int32_t m = 1U << (word_length - 1);
  int32_t x = (word) & ((1ULL << word_length) - 1);
  return ((x ^ m) - m);
}

static inline uint32_t rotate_right(uint32_t input, uint32_t shiftamount) {
  return (((uint32_t)input) >> shiftamount) |
         (((uint32_t)input) << (32 - shiftamount));
}

inline double load_double(uint32_t index) {
  double res;
  uint64_t input = ((uint64_t)RBD[index]) + (((uint64_t)RBD[index + 1]) << 32);
  memcpy(&res, &input, sizeof(input));
  return res;
}

inline void save_double(double input, uint32_t index) {
  uint64_t temp;
  memcpy(&temp, &input, sizeof(temp));
  RBD[index] = temp & 0xFFFFFFFF;
  RBD[index + 1] = temp >> 32;
}

inline float load_float(uint32_t index) {
  float res;
  memcpy(&res, &RBS[index], sizeof(uint32_t));
  return res;
}

inline void save_float(float input, uint32_t index) {
  memcpy(&RBS[index], &input, sizeof(uint32_t));
}

static bool custom_isnan(double var) {
  return var != var;
}

static bool custom_isnanf(float var) {
  return var != var;
}
