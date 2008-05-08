/**CFile****************************************************************

  FileName    [saigRetStep.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Sequential AIG package.]

  Synopsis    [Implementation of retiming steps.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: saigRetStep.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "saig.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Performs one retiming step forward.]

  Description [Returns the pointer to the register output after retiming.]
               
  SideEffects [Remember to run Aig_ManSetPioNumbers() in advance.]

  SeeAlso     []

***********************************************************************/
Aig_Obj_t * Saig_ManRetimeNodeFwd( Aig_Man_t * p, Aig_Obj_t * pObj )
{
    Aig_Obj_t * pFanin0, * pFanin1;
    Aig_Obj_t * pInput0, * pInput1;
    Aig_Obj_t * pObjNew, * pObjLi, * pObjLo;

    assert( Saig_ManRegNum(p) > 0 );
    assert( Aig_ObjIsNode(pObj) );

    // get the fanins
    pFanin0 = Aig_ObjFanin0(pObj);
    pFanin1 = Aig_ObjFanin1(pObj);
    // skip of they are not primary inputs
    if ( !Aig_ObjIsPi(pFanin0) || !Aig_ObjIsPi(pFanin1) )
        return NULL;

    // skip of they are not register outputs
    if ( !Saig_ObjIsLo(p, pFanin0) || !Saig_ObjIsLo(p, pFanin1) )
        return NULL;
    assert( Aig_ObjPioNum(pFanin0) > 0 );
    assert( Aig_ObjPioNum(pFanin1) > 0 );

    // get the inputs of these registers
    pInput0 = Saig_ManLi( p, Aig_ObjPioNum(pFanin0) - Saig_ManPiNum(p) );
    pInput1 = Saig_ManLi( p, Aig_ObjPioNum(pFanin1) - Saig_ManPiNum(p) );
    pInput0 = Aig_ObjChild0( pInput0 );
    pInput1 = Aig_ObjChild0( pInput1 );
    pInput0 = Aig_NotCond( pInput0, Aig_ObjFaninC0(pObj) );
    pInput1 = Aig_NotCond( pInput1, Aig_ObjFaninC1(pObj) );

    // create new node
    pObjNew = Aig_And( p, pInput0, pInput1 );

    // create new register input
    pObjLi = Aig_ObjCreatePo( p, Aig_NotCond(pObjNew, pObjNew->fPhase) );
    pObjLi->PioNum = Aig_ManPoNum(p) - 1;
    assert( pObjLi->fPhase == 0 );

    // create new register output
    pObjLo = Aig_ObjCreatePi( p );
    pObjLo->PioNum = Aig_ManPiNum(p) - 1;
    p->nRegs++;

    // return register output
    return Aig_NotCond( pObjLo, pObjNew->fPhase );
}

/**Function*************************************************************

  Synopsis    [Performs one retiming step backward.]

  Description [Returns the pointer to node after retiming.]
               
  SideEffects [Remember to run Aig_ManSetPioNumbers() in advance.]

  SeeAlso     []

***********************************************************************/
Aig_Obj_t * Saig_ManRetimeNodeBwd( Aig_Man_t * p, Aig_Obj_t * pObjLo )
{
    Aig_Obj_t * pFanin0, * pFanin1;
    Aig_Obj_t * pLo0New, * pLo1New;
    Aig_Obj_t * pLi0New, * pLi1New;
    Aig_Obj_t * pObj, * pObjNew, * pObjLi;
    int fCompl0, fCompl1;

    assert( Saig_ManRegNum(p) > 0 );
    assert( Aig_ObjPioNum(pObjLo) > 0 );
    assert( Saig_ObjIsLo(p, pObjLo) );

    // get the corresponding latch input
    pObjLi = Saig_ManLi( p, Aig_ObjPioNum(pObjLo) - Saig_ManPiNum(p) );

    // get the node
    pObj = Aig_ObjFanin0(pObjLi);
    if ( !Aig_ObjIsNode(pObj) )
        return NULL;

    // get the fanins
    pFanin0 = Aig_ObjFanin0(pObj);
    pFanin1 = Aig_ObjFanin1(pObj);

    // get the complemented attributes of the fanins
    fCompl0 = Aig_ObjFaninC0(pObj) ^ Aig_ObjFaninC0(pObjLi);
    fCompl1 = Aig_ObjFaninC1(pObj) ^ Aig_ObjFaninC0(pObjLi);

    // create latch inputs
    pLi0New = Aig_ObjCreatePo( p, Aig_NotCond(pFanin0, fCompl0) );
    pLi0New->PioNum = Aig_ManPoNum(p) - 1;
    pLi1New = Aig_ObjCreatePo( p, Aig_NotCond(pFanin1, fCompl1) );
    pLi1New->PioNum = Aig_ManPoNum(p) - 1;

    // create latch outputs
    pLo0New = Aig_ObjCreatePi(p);
    pLo0New->PioNum = Aig_ManPiNum(p) - 1;
    pLo1New = Aig_ObjCreatePi(p);
    pLo1New->PioNum = Aig_ManPiNum(p) - 1;
    pLo0New = Aig_NotCond( pLo0New, fCompl0 );
    pLo1New = Aig_NotCond( pLo1New, fCompl1 );
    p->nRegs += 2;

    // create node
    pObjNew = Aig_And( p, pLo0New, pLo1New );
//    assert( pObjNew->fPhase == 0 );
    return pObjNew;
}

/**Function*************************************************************

  Synopsis    [Performs the given number of retiming steps.]

  Description [Returns the pointer to node after retiming.]
               
  SideEffects [Remember to run Aig_ManSetPioNumbers() in advance.]

  SeeAlso     []

***********************************************************************/
void Saig_ManRetimeSteps( Aig_Man_t * p, int nSteps, int fForward )
{
    Aig_Obj_t * pObj, * pObjNew;
    int RetValue, s, i;
    Aig_ManSetPioNumbers( p );
    Aig_ManFanoutStart( p );
    if ( fForward )
    {
        for ( s = 0; s < nSteps; s++ )
        {
            Aig_ManForEachNode( p, pObj, i )
            {
                pObjNew = Saig_ManRetimeNodeFwd( p, pObj );
                if ( pObjNew == NULL )
                    continue;
                Aig_ObjReplace( p, pObj, pObjNew, 0, 0 );
                break;
            }
        }
    }
    else
    {
        for ( s = 0; s < nSteps; s++ )
        {
            Saig_ManForEachLo( p, pObj, i )
            {
                pObjNew = Saig_ManRetimeNodeBwd( p, pObj );
                if ( pObjNew == NULL )
                    continue;
                Aig_ObjReplace( p, pObj, pObjNew, 0, 0 );
                break;
            }
        }
    }
    RetValue = Aig_ManCleanup( p );
    assert( RetValue == 0 );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


