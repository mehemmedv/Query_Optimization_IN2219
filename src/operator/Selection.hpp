#ifndef H_operator_Selection
#define H_operator_Selection
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include <memory>
#include <vector>
//---------------------------------------------------------------------------
/// A selection
class Selection : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> input;
   /// Registers of the condition
   std::vector<const Register*> conditions;
   /// Second register for implicit equal tests
   std::vector<const Register*> equals;

   public:
   /// Constructor. Condition must be a bool value
   Selection(std::unique_ptr<Operator>&& input,const Register* condition);
   /// Constructor. Registers a and b are compared
   Selection(std::unique_ptr<Operator>&& input,const Register* a,const Register* b);
   
   /// Constructor. Condition must be a bool value
   Selection(std::unique_ptr<Operator>&& input,std::vector<const Register*> conditions);
   /// Constructor. Registers a and b are compared
   Selection(std::unique_ptr<Operator>&& input,std::vector<const Register*> as,std::vector<const Register*> bs);
   
   /// Destructor
   ~Selection();

   /// Open the operator
   void open();
   /// Get the next tuple
   bool next();
   /// Close the operator
   void close();

   /// Get all produced values
   std::vector<const Register*> getOutput() const;
};
//---------------------------------------------------------------------------
#endif
