#include "operator/Selection.hpp"
#include "Register.hpp"
#include "IteratorTools.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
Selection::Selection(unique_ptr<Operator>&& input,const Register* condition)
   : input(move(input)),conditions({condition}),equals({NULL})
   // Constructor
{
}
//---------------------------------------------------------------------------
Selection::Selection(unique_ptr<Operator>&& input,const Register* a,const Register* b)
   : input(move(input)),conditions({a}),equals({b})
   // Constructor
{
}
//---------------------------------------------------------------------------
Selection::Selection(unique_ptr<Operator>&& input,vector<const Register*> conditions)
   : input(move(input)),conditions(move(conditions)),equals(conditions.size(), NULL)
   // Constructor
{
}
//---------------------------------------------------------------------------
Selection::Selection(unique_ptr<Operator>&& input,vector<const Register*> as,vector<const Register*> bs)
   : input(move(input)),conditions(move(as)),equals(move(bs))
   // Constructor
{
}
//---------------------------------------------------------------------------
Selection::~Selection()
   // Destructor
{
}
//---------------------------------------------------------------------------
void Selection::open()
   // Open the operator
{
   input->open();
}
//---------------------------------------------------------------------------
bool Selection::next()
   // Get the next tuple
{
   while (true) {
      // Produce a tuple
      if (!input->next())
         return false;
      // Check
      
      bool truth = true;
      for (auto zippair : Zip<const Register*>(conditions, equals)) {
         auto& condition = zippair.first;
         auto& equal = zippair.second;
         
         if (equal) {
            if (condition->getState()==equal->getState()) switch (condition->getState()) {
               case Register::State::Unbound: break;
               case Register::State::Int: truth = truth && (condition->getInt()==equal->getInt()); break;
               case Register::State::Double: truth = truth && (condition->getDouble()==equal->getDouble()); break;
               case Register::State::Bool: truth = truth && (condition->getBool()==equal->getBool()); break;
               case Register::State::String: truth = truth && (condition->getString()==equal->getString()); break;
            }
         } else {
            truth = truth && ((condition->getState()==Register::State::Bool)&&(condition->getBool()));
         }
      }
      
      if (truth) {
         return true;
      }
   }
}
//---------------------------------------------------------------------------
void Selection::close()
   // Close the operator
{
   input->close();
}
//---------------------------------------------------------------------------
vector<const Register*> Selection::getOutput() const
   // Get all produced values
{
   return input->getOutput();
}
//---------------------------------------------------------------------------
