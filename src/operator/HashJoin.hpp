#ifndef H_operator_HashJoin
#define H_operator_HashJoin
//---------------------------------------------------------------------------
#include "operator/Operator.hpp"
#include "Register.hpp"
#include <memory>
#include <unordered_map>
//---------------------------------------------------------------------------
/// A hash join
class HashJoin : public Operator
{
   private:
   /// The input
   std::unique_ptr<Operator> left,right;
   /// The registers
   std::vector<const Register*> leftValues,rightValues;
   /// The copy mechanism
   std::vector<Register*> leftRegs;
   /// The hashtable
   struct registerVectorHash { 
      unsigned operator()(const std::vector<Register>& rs) const { 
         std::size_t seed = rs.size();
         for(auto& r : rs) {
            seed ^= r.getHash() + 0x9e3779b9 + (seed << 6) + (seed >> 2);  //boost hash_combine
         }
         return seed;
      }; 
   };
   typedef std::unordered_multimap<std::vector<Register>,std::vector<Register>,registerVectorHash> hashtable;
   hashtable table;
   /// Iterator
   hashtable::const_iterator iter,iterLimit;

   public:
   /// Constructor
   HashJoin(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right,const Register* leftValue,const Register* rightValue);
   HashJoin(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right,std::vector<const Register*> leftValues,std::vector<const Register*> rightValues);
   /// Destructor
   ~HashJoin();

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
