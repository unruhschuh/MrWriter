#ifndef NAMESPACE_NAMES_THEN_PRIMARY_CLASS_OR_FUNCTION_THEN_HPP
#define NAMESPACE_NAMES_THEN_PRIMARY_CLASS_OR_FUNCTION_THEN_HPP

#include <boost/headers/go/first>
#include <boost/in_alphabetical/order>
#include <then_standard_headers>
#include <in_alphabetical_order>

#include "then/any/detail/headers"
#include "in/alphabetical/order"
#include "then/any/remaining/headers/in"
#include "alphabetical/order/duh"

#define NAMESPACE_NAMES_THEN_MACRO_NAME(p_macroNames) ARE_ALL_CAPS

namespace NameSpaceInCamelCase
{
  class ClassesAreAlsoCamels
  {

/*##############################################################################
# PUBLIC FUNCTIONS
##############################################################################*/
  public:

    void functionsUsePascalCase() const
    {
      volatile int localVariablesUsePascalCase = 0;
    }

    const complex_type& memberVariable() const
    {
      return m_memberVariable; // no conflict with value here
    }

    void setMemberVariable(const SomeType& p_parameterVariable)
    {
      m_memberVariable = p_parameterVariable; // or here
    }


/*##############################################################################
# PUBLIC MEMBERS
##############################################################################*/
  public:

    int m_iAmAPublicMember;

  // the more public it is, the more important it is,
  // so order: public on top, then protected then private

/*******************************************************************************
* PROTECTED FUNCTIONS
*******************************************************************************/
  protected:

  template <typename Template, typename Parameters>
  void areCamelCase()
  {
    // ...
  }

/*******************************************************************************
* PROTECTED MEMBERS
*******************************************************************************/
  protected:

/*------------------------------------------------------------------------------
| PRIVATE FUNCTIONS
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
| PRIVATE MEMBERS
------------------------------------------------------------------------------*/
  private:

    SomeType m_memberVariable;
  };
}

#endif
