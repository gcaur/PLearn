

#ifndef CopiesMap_INC
#define CopiesMap_INC

//! Macro to define deep copy for types that actually do not require
//! any deep copy (such as int, real, etc.).
#define NODEEPCOPY(TYPE)                              \
      inline void deepCopyField(TYPE&, CopiesMap&) {}

#include <map>
#include <string>
#include "plerror.h"    //!< For PLWARNING.

namespace PLearn {

class VMField;
class VMFieldStat;

using namespace std;

  //!  Global typedef to make the map of copied objects (needed by the deep
  //!  copy mechanism in Object) more palatable
  typedef map<const void*,void*> CopiesMap;

  
/*! Support for generic deep copying
    
    Deep copying is defined for objects in the following manner:
    + copy constructors should always do a shallow copy.
    + a public method OBJTYPE* deepCopy(map<const void*, void*>& copies) const 
      should be defined to allow deepCopying
    + the deepCopy method should be virtual for classes that are designed to be subclassed
    Take a close look at the Object class in Object.h to see how this is done.
*/

  //! Types that do not require deep copy.
  NODEEPCOPY(double)
  NODEEPCOPY(float)
  NODEEPCOPY(int)
  NODEEPCOPY(bool)
  NODEEPCOPY(string)
  NODEEPCOPY(VMField)
  NODEEPCOPY(VMFieldStat)

  //!  Any type not handled below: do nothing
  template <class T>
  inline void deepCopyField(T&, CopiesMap&)
  {
    /*! no op */
    PLWARNING(
        "In CopiesMap.h - deepCopyField not handled for this type. "
        "If it actually doesn't need deep copy, edit CopiesMap.h and add NODEEPCOPY(your_type) to remove this warning."
    );
  }

  template <class T>
  inline void deepCopyField(T*& field, CopiesMap& copies)
  {
    if (field)
      field = field->deepCopy(copies);
  }

  //!  A simple template function that calls the method
  template<class T>
  T* deepCopy(const T* source, CopiesMap& copies)
  { return source->deepCopy(copies); }

//!  This function simply calls the previous one with an initially empty map
template<class T>
inline T* deepCopy(const T* source)
{ 
  CopiesMap copies; //!<  create empty map
  return deepCopy(source, copies);
}


}

#endif //ndef CopiesMap_INC
