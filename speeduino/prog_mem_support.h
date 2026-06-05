/**
 * @file
 * @brief Helper methods for items stored in flash memory
 */
#pragma once

/**
 * @brief Copy an object stored in flash to an existing RAM based instance
 * 
 * @tparam T Type of object to load. **Must be a POD**
 * @param pAddress Address to load from
 * @param t Object instance to load into. 
 * @return T& Reference to t
 */
template <typename T>
static inline T& copyObject_P(const T *pAddress, T &t)
{
  (void)memcpy_P(&t, pAddress, sizeof(T));
  return t;
}

/**
 * @brief Copy an object stored in flash to a new instance in RAM
 * 
 * @tparam T Type of object to load. **Must be a POD**
 * @param pAddress Address to load from
 * @return T Loaded object
 */
template <typename T>
static inline T copyObject_P(const T *pAddress)
{
  T t = {};
  return copyObject_P(pAddress, t);
}