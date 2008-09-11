
#include <anafold/module/anafold.h>
#include <si/global/global.h>
#include <qtranslator.h>

using namespace anatomist;
using namespace sigraph;
using namespace std;


static bool initAnaFold()
{
  AnaFold	*a = new AnaFold;
  a->init();
  return( true );
}

static bool garbage = initAnaFold();


AnaFold::AnaFold() : Module()
{
  // force using sigraph library (needed on Mac-Darwin)
  si();
}


AnaFold::~AnaFold()
{
}


string AnaFold::name() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "Folds recognition" ) );
}


string AnaFold::description() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", 
			     "Cortical folds recognition package" ) );
}


void AnaFold::objectsDeclaration()
{
}


