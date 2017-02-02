
// Placement new
// Operator new allocates memory from the heap, on which an object is 
// constructed. Standard C++ also supports placement new operator, 
// which constructs an object on a pre-allocated buffer. This is useful 
// when building a memory pool, a garbage collector or simply when 
// performance and exception safety are paramount (there's no danger of 
// allocation failure since the memory has already been allocated, and 
// constructing an object on a pre-allocated buffer takes less time):
 

// If you enable this option and want to put a dot in the middle of a sentence
//  without ending it, you should put a backslash and a space after it.



#if 0
/*! \brief Brief 1 description.
 *         Brief description continued.
 *
 *  Detailed 1 description starts here.
 */
#endif


/// Brief 2 description which ends at this dot. Details follow
/// here.
/// 


class Test
{
  public:

#if 0
    /** 
     * An enum.
     * More detailed enum description.
     */
#endif

    //! 
    //! An enum.
    //!	
    //! More detailed enum description.
    //!
    enum TEnum { 
          TVal1, /**< enum value TVal1. */  
          TVal2, /**< enum value TVal2. */  
          TVal3  /**< enum value TVal3. */  
         } 
       *enumPtr, /**< enum pointer. Details. */
       enumVar;  /**< enum variable. Details. */
       
      /**
       * A constructor.
       * A more elaborate description of the constructor.
       */
      Test();

      /**
       * A destructor.
       * A more elaborate description of the destructor.
       */
     ~Test();
    
      /**
       * a normal member taking two arguments and returning an integer value.
       * @param a an integer argument.
       * @param s a constant character pointer.
       * @see Test()
       * @see ~Test()
       * @see testMeToo()
       * @see publicVar()
       * @return The test results
       */
       int testMe(int a,const char *s);
       
      /**
       * A pure virtual member.
       * @see testMe()
       * @param c1 the first argument.
       * @param c2 the second argument.
       */
       virtual void testMeToo(char c1,char c2) = 0;
   
      /** 
       * a public variable.
       * Details.
       */
       int publicVar;
       
      /**
       * a function variable.
       * Details.
       */
       int (*handler)(int a,int b);
};


class TestB 
{


  void placement() {
    char *buf  = new char[1000];   //pre-allocated buffer
    Test *p = new (buf) Test();    //placement new
  }

  void normalalloc() {
    Test *p = new Test();    //normal new
    p->testMe(100, "test string");
  }

};
    

