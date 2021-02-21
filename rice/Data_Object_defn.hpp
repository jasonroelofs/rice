#ifndef Rice__Data_Object_defn__hpp_
#define Rice__Data_Object_defn__hpp_

#include "detail/to_ruby.hpp"
#include "detail/ruby.hpp"
#include "Object_defn.hpp"

/*! \file
 *  \brief Provides a helper class for wrapping and unwrapping C++
 *  objects as Ruby objects.
 */

namespace Rice
{

//! A smartpointer-like wrapper for Ruby data objects.
/*! A data object is a ruby object of type T_DATA, which is usually
 *  created by using the Data_Wrap_Struct or Data_Make_Struct macro.
 *  This class wraps creation of the data structure, providing a
 *  type-safe object-oriented interface to the underlying C interface.
 *  This class works in conjunction with the Data_Type class to ensure
 *  type safety.
 *
 *  Example:
 *  \code
 *    class Foo { };
 *    ...
 *    Data_Type<Foo> rb_cFoo = define_class("Foo");
 *    ...
 *    // Wrap:
 *    Data_Object<Foo> foo1(new Foo);
 *
 *    // Get value to return:
 *    VALUE v = foo1.value()
 *
 *    // Unwrap:
 *    Data_Object<Foo> foo2(v, rb_cFoo);
 *  \endcode
 */
template<typename T>
class Data_Object
  : public Object
{
  static_assert(!std::is_pointer_v<T>);
  static_assert(!std::is_reference_v<T>);
  static_assert(!std::is_const_v<T>);
  static_assert(!std::is_volatile_v<T>); 

public:
  //! A function that takes a T* and returns void.
  typedef void (*Ruby_Data_Func)(T* obj);
  typedef void (*Ruby_Free_Func)(detail::Wrapper<T>* obj);

  //! Wrap a C++ object.
  /*! This constructor is analogous to calling Data_Wrap_Struct.  Be
   *  careful not to call this function more than once for the same
   *  pointer (in general, it should only be called for newly
   *  constructed objects that need to be managed by Ruby's garbage
   *  collector).
   *  \param obj the object to wrap.
   *  \param klass the Ruby class to use for the newly created Ruby
   *  object.
   *  \param mark_func a function that gets called by the garbage
   *  collector to mark the object's children.
   *  \param free_func a function that gets called by the garbage
   *  collector to free the object.
   */
  Data_Object(T* obj, Module klass = Data_Type<T>::klass());

  //! Unwrap a Ruby object.
  /*! This constructor is analogous to calling Data_Get_Struct.  Uses
   *  Data_Type<T>::klass as the class of the object.
   *  \param value the Ruby object to unwrap.
   */
  Data_Object(
      Object value);

  //! Unwrap a Ruby object.
  /*! This constructor is analogous to calling Data_Get_Struct.  Will
   *  throw an exception if the class of the object differs from the
   *  specified class.
   *  \param value the Ruby object to unwrap.
   *  \param klass the expected class of the object.
   */
  template<typename U>
  Data_Object(Object value);

  T& operator*() const; //!< Return a reference to obj_
  T* operator->() const; //!< Return a pointer to obj_
  T* get() const;        //!< Return a pointer to obj_

private:
  static void check_ruby_type(VALUE value);
};

} // namespace Rice

#endif // Rice__Data_Object_defn__hpp_

