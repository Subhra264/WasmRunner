#include "functional"

class finally
{
   std::function<void(void)> mFunctor;
   public:
      finally(const std::function<void(void)> &functor) : mFunctor(functor) {}
      ~finally()
      {
         mFunctor();
      }
};