/********************************************************************************/
/*     888888    888888888   88     888  88888   888      888    88888888       */
/*   8       8   8           8 8     8     8      8        8    8               */
/*  8            8           8  8    8     8      8        8    8               */
/*  8            888888888   8   8   8     8      8        8     8888888        */
/*  8      8888  8           8    8  8     8      8        8            8       */
/*   8       8   8           8     8 8     8      8        8            8       */
/*     888888    888888888  888     88   88888     88888888     88888888        */
/*                                                                              */
/*       A Three-Dimensional General Purpose Semiconductor Simulator.           */
/*                                                                              */
/*                                                                              */
/*  Copyright (C) 2007-2008                                                     */
/*  Cogenda Pte Ltd                                                             */
/*                                                                              */
/*  Please contact Cogenda Pte Ltd for license information                      */
/*                                                                              */
/*  Author: Gong Ding   gdiso@ustc.edu                                          */
/*                                                                              */
/********************************************************************************/

#include <cstdlib>
#include <iomanip>

#include "solver_base.h"
#include "cgns_hook.h"
#include "spice_ckt.h"
#include "MXMLUtil.h"


/*----------------------------------------------------------------------
 * constructor, open the file for writing
 */
CGNSHook::CGNSHook ( SolverBase & solver, const std::string & name, void * param)
    : Hook ( solver, name ), _cgns_prefix ( SolverSpecify::out_prefix ), _mixA ( false )
{
  this->count  =0;
  this->_t_step=0;
  this->_v_step=0;
  this->_i_step=0;
  this->_t_last=0;
  this->_v_last=0;
  this->_i_last=0;


  this->_t_start=0;
  this->_t_stop =std::numeric_limits<double>::infinity();

  const std::vector<Parser::Parameter> & parm_list = *((std::vector<Parser::Parameter> *)param);
  for ( std::vector<Parser::Parameter>::const_iterator parm_it = parm_list.begin();
        parm_it != parm_list.end(); parm_it++ )
  {
    if ( parm_it->name() == "tstep" && parm_it->type() == Parser::REAL )
      _t_step=parm_it->get_real() * PhysicalUnit::s;
    if ( parm_it->name() == "vstep" && parm_it->type() == Parser::REAL )
      _v_step=parm_it->get_real() * PhysicalUnit::V;
    if ( parm_it->name() == "istep" && parm_it->type() == Parser::REAL )
      _i_step=parm_it->get_real() * PhysicalUnit::A;

    if ( parm_it->name() == "tstart" && parm_it->type() == Parser::REAL )
      _t_start=parm_it->get_real() * PhysicalUnit::s;
    if ( parm_it->name() == "tstop" && parm_it->type() == Parser::REAL )
      _t_stop=parm_it->get_real() * PhysicalUnit::s;
  }


  const SimulationSystem &system = get_solver().get_system();

  std::ostringstream cgns_filename;
  cgns_filename << _cgns_prefix << '.' << ( this->count++ ) << ".cgns";
  system.export_cgns ( cgns_filename.str() );

  SolverSpecify::SolverType solver_type = this->get_solver().solver_type();

  // if we are called by mixA solver?
  switch ( solver_type )
  {
    case SolverSpecify::DDML1     :
    case SolverSpecify::DDML2     :
    case SolverSpecify::EBML3     :   _ddm = true; break;
    case SolverSpecify::DDML1MIXA :
    case SolverSpecify::DDML2MIXA :
    case SolverSpecify::EBML3MIXA :   _mixA = true; break;
    case SolverSpecify::DDMAC     :   _ddm_ac = true; break;
    default : break;
  }
}


/*----------------------------------------------------------------------
 * destructor, close file
 */
CGNSHook::~CGNSHook()
{}


/*----------------------------------------------------------------------
 *   This is executed before the initialization of the solver
 */
void CGNSHook::on_init()
{

}



/*----------------------------------------------------------------------
 *   This is executed previously to each solution step.
 */
void CGNSHook::pre_solve()
{}



/*----------------------------------------------------------------------
 *  This is executed after each solution step.
 */
void CGNSHook::post_solve()
{

  std::ostringstream cgns_filename;
  if ( SolverSpecify::Type==SolverSpecify::DCSWEEP && SolverSpecify::Electrode_VScan.size() )
  {
    double Vscan = SolverSpecify::Electrode_VScan_Voltage;

    if ( std::fabs ( Vscan - this->_v_last ) >= this->_v_step )
    {
      const SimulationSystem &system = get_solver().get_system();

      cgns_filename << _cgns_prefix << '.' << ( this->count++ ) << ".cgns";
      system.export_cgns ( cgns_filename.str() );

      _v_last = Vscan;
    }
  }

  if ( SolverSpecify::Type==SolverSpecify::DCSWEEP && SolverSpecify::Electrode_IScan.size() )
  {
    double Iscan = SolverSpecify::Electrode_IScan_Current;

    if ( std::fabs ( Iscan - this->_i_last ) >= this->_i_step )
    {
      const SimulationSystem &system = get_solver().get_system();

      cgns_filename << _cgns_prefix << '.' << ( this->count++ ) << ".cgns";
      system.export_cgns ( cgns_filename.str() );

      _i_last = Iscan;
    }
  }


  if ( SolverSpecify::Type==SolverSpecify::TRANSIENT )
  {
    if(SolverSpecify::clock >= _t_start && SolverSpecify::clock <= _t_stop )
    {
      if ( SolverSpecify::clock - this->_t_last >= this->_t_step )
      {
        const SimulationSystem &system = get_solver().get_system();

        cgns_filename << _cgns_prefix << '.' << ( this->count++ ) << ".cgns";
        system.export_cgns ( cgns_filename.str() );

        _t_last = SolverSpecify::clock;

      }
    }
  }

  ////----
  if ( !cgns_filename.str().empty() )
  {
    mxml_node_t *eSolution = get_solver().current_dom_solution_elem();
    if ( eSolution )
    {
      mxml_node_t *eOutput  = mxmlFindElement ( eSolution, eSolution, "output", NULL, NULL, MXML_DESCEND_FIRST );
      mxml_node_t *eCGNS = mxmlNewElement ( eOutput, "cgns" );
      mxml_node_t *eFile    = mxmlNewElement ( eCGNS, "file" );
      mxmlAdd ( eFile, MXML_ADD_AFTER, NULL, MXMLQVariant::makeQVString ( cgns_filename.str() ) );
    }
  }

}



/*----------------------------------------------------------------------
 *  This is executed after each (nonlinear) iteration
 */
void CGNSHook::post_iteration()
{}



/*----------------------------------------------------------------------
 * This is executed after the finalization of the solver
 */
void CGNSHook::on_close()
{}


#ifdef DLLHOOK

// dll interface
extern "C"
{
  Hook* get_hook ( SolverBase & solver, const std::string & name, void * fun_data )
  {
    return new CGNSHook ( solver, name, fun_data );
  }

}

#endif

