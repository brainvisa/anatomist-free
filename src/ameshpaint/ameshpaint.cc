//AimsMeshPaint -im /home/arnaud/Bureau/vrac/AYo1_t2_nobias_cortex_hemil.mesh -it /home/arnaud/Bureau/vrac/AYo1_blobs.tex -ic blue_red_bis.rgb -ot /home/arnaud/Bureau/vrac/texout.gii
//AimsMeshPaint -im /home/arnaud/Bureau/vrac/subject01_Lwhite_inflated.mesh -it /home/arnaud/Bureau/vrac/subject01_Lwhite_curv.tex -ic blue_red_bis.rgb -ot /home/arnaud/Bureau/vrac/texout.gii
//anatomist --userLevel 4  /home/arnaud/Bureau/vrac/subject01_Lwhite_inflated.mesh /home/arnaud/Bureau/vrac/subject01_Lwhite_curv.tex
#include "meshpaint.h"

int main(int argc, const char **argv)
{
  //DECLARATIONS
  std::string adressTexIn="./";
  std::string adressMeshIn="./";
  std::string adressTexOut="./";
  std::string colorMap="blue_red_bis.rgb";

  AimsApplication     app( argc, argv, "MeshPaint : draw a texture on mesh");

  try
  {
    app.addOption( adressMeshIn, "-im", "input mesh");
    app.alias( "--inputMesh", "-im" );
    app.addOption( adressTexIn, "-it", "input texture");
    app.alias( "--inputTex", "-it" );
    app.addOption( colorMap, "-ic", "input colormap (blue_red_bis.rgb by default)",true);
    app.alias( "--inputColorMap", "-ic" );
    app.addOption( adressTexOut, "-ot", "output texture");
    app.alias( "--outputTex", "-ot" );
    app.initialize();
    }
  catch( user_interruption & )
  {
    return EXIT_FAILURE;
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    return EXIT_FAILURE;
  }

  Finder	f;
  f.check( adressTexIn );
  string	type = f.dataType();

  QApplication a(argc, (char **)argv);

  #ifdef QT_OPENGL_SUPPORT
      QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
  #endif

  if (type == "FLOAT")
  {
    myMeshPaint<float> wf(adressTexIn,adressMeshIn,adressTexOut,colorMap,type);
    a.exec();
  }

  if (type == "S16")
  {
    myMeshPaint<short> ws(adressTexIn,adressMeshIn,adressTexOut,colorMap,type);
    a.exec();
  }

  return 1;
}

