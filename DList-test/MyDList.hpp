#ifndef DList_hpp
#define DList_hpp
/*******************************************************************************
*  DList.hpp
*
*  Copyright (C) 2002-2005 Arroyo Video Solutions, Inc.
*  All Rights Reserved.
*  
*******************************************************************************/

//#include "CalypsoDefines.hpp"

#ifdef __KERNEL__
   #include "CalypsoTypes.hpp"
   #include "calypso_debug.hpp"
   #include "new.hpp"
   #define DListAssert  ASSERT
#else
#if GCC_4_PLUS
   // stdlib.h uses __init as a structure member.
   #undef __init
#endif // GCC_4_PLUS
   #include <stdlib.h>
   #include <assert.h>

   #define DListAssert  assert
#endif


/*******************************************************************************
*  Definitions
*******************************************************************************/

// From CalypsoDefines.hpp
#if GCC_4_PLUS
#define UTIL_FIELD_OFFSET(className, fieldName)  \
      __builtin_offsetof(className, fieldName)
#else // !GCC_4_PLUS
#define UTIL_FIELD_OFFSET(className, fieldName)  \
      (((unsigned int) &((className *)1)->fieldName) - 1)
#endif // GCC_4_PLUS



#define DLIST_TYPE(className,nodeField)   \
    DList<className,UTIL_FIELD_OFFSET(className,nodeField)>

#define COUNTED_DLIST_TYPE(className,nodeField)   \
    CountedDList<className,UTIL_FIELD_OFFSET(className,nodeField)>


/*******************************************************************************
*  Types and Classes
*******************************************************************************/

//------------------------------------------------------------------------------
// DListNode
//
// Note that DListNode does not remove itself from its list when deleted.
//
class DListNode
{
   public:

   DListNode () : m_next (NULL) {}

   bool isOnList () { return m_next != NULL; }

   DListNode * m_next;
   DListNode * m_prev;
};


//------------------------------------------------------------------------------
// DList
//
template <class T, uint nodeOffset>
class DList
{
   //---------------------------------------------------------------------------
   // Public interface
   //
   public:

   typedef int TFunc (T *, void * context);

   DList ();
   ~DList ();

   void addHead (T *);
   void addTail (T *);

   static void insertAfter (T * afterHere, T * newObject);
   static void insertBefore (T * beforeHere, T * newObject);

   T * getHead ();
   T * getTail ();
   T * removeHead ();

   static T *  getNext (T *);
   static T *  getPrev (T *);
   static void remove (T *);

   void removeAll ();

   bool isEmpty ();
   bool isMember (T * t);
   void deleteItems ();
   void moveList (DList * sourceList);
   int foreach (TFunc, void *);
   void swap (T* item1, T* item2);        // swap two items on the same list

   T * Null ();              // Callers ** MUST ** test against this, not NULL

   //---------------------------------------------------------------------------
   // These functions are not normally needed publicly. Only for highly
   // optimized list traversal.  See foreach().
   //
   DListNode * getHeadNode () { return listHead.m_next; }
   DListNode * getTailNode () { return listHead.m_prev; }
   DListNode * endOfList ()   { return &listHead; }

   static DListNode * getNextNode (DListNode * n) { return n->m_next; }
   static DListNode * getPrevNode (DListNode * n) { return n->m_prev; }
   static T * getObject (DListNode * n);

   
   //---------------------------------------------------------------------------
   // Protected interface
   //
   protected:

   static DListNode * getNode (T * t);
   static void insertAfter (DListNode * afterHere, DListNode * n);
   static void insertBefore (DListNode * beforeHere, DListNode * n);
   static void remove (DListNode * n);

   //---------------------------------------------------------------------------
   // Protected data
   // 
   DListNode   listHead;
};


//------------------------------------------------------------------------------
// CountedDList
//
template <class T, uint nodeOffset>
class CountedDList : public DList<T, nodeOffset>
{
   //---------------------------------------------------------------------------
   // Public interface
   //
   public:

   CountedDList ();

   void addHead (T *);
   void addTail (T *);

   T * removeHead ();

   // This is not valid for CountedDList; m_nodeCount cannot be maintained
   // Intentionally left unimplemented to cause link error.
   static void remove (T *);

   void remove (T *);

   void deleteItems ();
   void moveList (CountedDList * sourceList);
#if GCC_4_PLUS
   T * Null ();              // Callers ** MUST ** test against this, not NULL
#endif // GCC_4_PLUS
   //---------------------------------------------------------------------------
   // Protected interface
   //
   protected:

   unsigned int   m_nodeCount;
};


/*******************************************************************************
*  Exported Globals
*******************************************************************************/


/*******************************************************************************
*  Exported Functions
*******************************************************************************/


/******************************************************************************/

template <class T, uint nodeOffset>
DList<T,nodeOffset>::DList ()
{
   listHead.m_next = endOfList ();
   listHead.m_prev = endOfList ();
}


/******************************************************************************/

template <class T, uint nodeOffset>
DList<T,nodeOffset>::~DList ()
{
   DListAssert (isEmpty ());
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::addHead (T * t)
{
   insertAfter (&listHead, getNode (t));
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::addTail (T * t)
{
   insertBefore (&listHead, getNode (t));
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::insertAfter (T * afterHere, T * newObject)
{
   insertAfter (getNode (afterHere), getNode (newObject));
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::insertBefore (T * beforeHere, T * newObject)
{
   insertBefore (getNode (beforeHere), getNode (newObject));
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::getHead ()
{
   return getObject (listHead.m_next);
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::getNext (T * t)
{
   return getObject (getNode(t)->m_next);
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::getPrev (T * t)
{
   return getObject (getNode(t)->m_prev);
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::getTail ()
{
   return getObject (listHead.m_prev);
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::remove (T * t)
{
   remove (getNode (t));
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::removeAll ()
{
   DListNode * n;
   DListNode * next;

   for (n = listHead.m_next; n != endOfList(); n = next)
   {
      next = n->m_next;
      remove (n);
   }
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::remove (DListNode * n)
{
   DListNode * next;
   DListNode * prev;

   next = n->m_next;
   prev = n->m_prev;

   next->m_prev = prev;
   prev->m_next = next;

   n->m_next = NULL;
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::removeHead ()
{
   T * t;

   t = getObject (listHead.m_next);

   if (t != Null())
   {
      remove (listHead.m_next);
   }
   return t;
}


/******************************************************************************/

template <class T, uint nodeOffset>
bool DList<T,nodeOffset>::isEmpty ()
{
   return listHead.m_next == endOfList();
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::Null ()
{
   return getObject (&listHead);
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::deleteItems ()
{
   DListNode * n;
   DListNode * next;

   for (n = listHead.m_next; n != endOfList(); n = next)
   {
      next = n->m_next;
      delete getObject (n);
   }

   listHead.m_next = endOfList ();
   listHead.m_prev = endOfList ();
}


/******************************************************************************/

template <class T, uint nodeOffset>
int DList<T,nodeOffset>::foreach (TFunc f, void * context = NULL)
{
   DListNode * n;
   int         result;;

   for (n = getHeadNode (); n != endOfList(); n = getNextNode (n))
   {
      result = f (getObject (n), context);
      if (result != 0)
      {
         return result;
      }
   }

   return 0;
}


/******************************************************************************/

template <class T, uint nodeOffset>
bool DList<T,nodeOffset>::isMember (T * t)
{
   DListNode * n;

   for (n = getHeadNode (); n != endOfList(); n = getNextNode (n))
   {
      if (getObject (n) == t)
      {
         return true;
      }
   }

   return false;
}


/******************************************************************************/

template <class T, uint nodeOffset>
DListNode * DList<T,nodeOffset>::getNode (T * t)
{
   return (DListNode *) (((char *) t) + nodeOffset);
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * DList<T,nodeOffset>::getObject (DListNode * n)
{
   return (T *) (((char *) n) - nodeOffset);
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::insertAfter (DListNode * afterHere, DListNode * n)
{
   DListNode * next = afterHere->m_next;

   n->m_next = next;
   n->m_prev = afterHere;
   afterHere->m_next = n;
   next->m_prev= n;
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::insertBefore (DListNode * beforeHere, DListNode * n)
{
   DListNode * prev = beforeHere->m_prev;

   n->m_next = beforeHere;
   n->m_prev = prev;
   prev->m_next = n;
   beforeHere->m_prev = n;
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::swap (T* item1, T* item2)
{
   T* prev1;
   T* prev2;

   if (item1 == item2)
   {
      return;
   }

   prev1 = getPrev (item1);
   prev2 = getPrev (item2);

   if (prev1 == item2)           // item2 is immediately before item1
   {
      remove (item2);
      insertAfter (item1, item2);
      return;
   }

   if (prev2 == item1)           // item1 is immediately before item2
   {
      remove (item1);
      insertAfter (item2, item1);
      return;
   }

   remove (item1);
   remove (item2);

   insertAfter (prev1, item2);
   insertAfter (prev2, item1);
}


/******************************************************************************/

template <class T, uint nodeOffset>
void DList<T,nodeOffset>::moveList (DList * sourceList)
{
   DListNode * sourceHead = sourceList->getHeadNode ();

   if (sourceHead != sourceList->endOfList ())
   {
      DListNode * sourceTail = sourceList->getTailNode ();
      DListNode * beforeHere = &listHead;
      DListNode * prev = beforeHere->m_prev;

      sourceTail->m_next = beforeHere;
      sourceHead->m_prev = prev;
      prev->m_next = sourceHead;
      beforeHere->m_prev = sourceTail;
   }
}


/******************************************************************************/

template <class T, uint nodeOffset>
CountedDList<T,nodeOffset>::CountedDList ()
{
   m_nodeCount = 0;
}

/******************************************************************************/

template <class T, uint nodeOffset>
void CountedDList<T,nodeOffset>::addHead (T * t)
{
   DList<T,nodeOffset>::addHead (t);
   m_nodeCount++;
}


/******************************************************************************/

template <class T, uint nodeOffset>
void CountedDList<T,nodeOffset>::addTail (T * t)
{
   DList<T,nodeOffset>::addTail (t);
   m_nodeCount++;
}


/******************************************************************************/

template <class T, uint nodeOffset>
T * CountedDList<T,nodeOffset>::removeHead ()
{
   T * t = DList<T,nodeOffset>::removeHead (t);
   if (t != Null())
   {
      m_nodeCount--;
   }
   return t;
}


/******************************************************************************/

template <class T, uint nodeOffset>
void CountedDList<T,nodeOffset>::remove (T * t)
{
   DListNode * n = getNode (t);

   if (n->m_next != NULL)
   {
      m_nodeCount--;
   }
   remove (n);
}


/******************************************************************************/
/******************************************************************************/
#endif  // DList_hpp

