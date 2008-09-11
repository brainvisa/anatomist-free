/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */


#include <anatomist/color/Light.h>

#include <anatomist/misc/error.h>


using namespace anatomist;


Light::Light()
{
  _ambient[0] = _ambient[1] = _ambient[2] = 0.0;
  _ambient[3] = 0.0;
  _diffuse[0] = _diffuse[1] = _diffuse[2] = _diffuse[3] = 1.0;
  _specular[0] = _specular[1] = _specular[2] = _specular[3] = 1.0;
  _position[0] = _position[1] = _position[3] = 0.0;
  _position[2] = 1.0;
  _spotDirection[0] = _spotDirection[1] = 0.0;
  _spotDirection[2] = -1.0;
  _spotExponent = 0.0;
  _spotCutoff = 180.0;
  _constantAttenuation = 1.0;
  _linearAttenuation = 0.0;
  _quadraticAttenuation = 0.0;
  _modelAmbient[0] = _modelAmbient[1] = _modelAmbient[2] = 0.2;
  _modelAmbient[3] = 1.0;
  _modelLocalViewer = 0.0;
  _modelTwoSide = 0.0;
  _background[0] = _background[1] = _background[2] = 1.0;
  _background[3] = 1.0;

  _GLList = 0;
  _GLLRefreshFlag = true;
}

Light::~Light()
{
  if (glIsList(_GLList)) glDeleteLists(_GLList,1);  
}

void Light::SetAmbient(float r, float g, float b, float a)
{
  _ambient[0] = r;
  _ambient[1] = g;
  _ambient[2] = b;
  _ambient[3] = a;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetDiffuse(float r, float g, float b, float a)
{
  _diffuse[0] = r;
  _diffuse[1] = g;
  _diffuse[2] = b;
  _diffuse[3] = a;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpecular(float r, float g, float b, float a)
{
  _specular[0] = r;
  _specular[1] = g;
  _specular[2] = b;
  _specular[3] = a;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetBackground(float r, float g, float b, float a)
{
  _background[0] = r;
  _background[1] = g;
  _background[2] = b;
  _background[3] = a;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetPosition(float x, float y, float z, float w)
{
  _position[0] = x;
  _position[1] = y;
  _position[2] = z;
  _position[3] = w;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotDirection(float x, float y, float z)
{
  _spotDirection[0] = x;
  _spotDirection[1] = y;
  _spotDirection[2] = z;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelAmbient(float r, float g, float b, float a)
{
  _modelAmbient[0] = r;
  _modelAmbient[1] = g;
  _modelAmbient[2] = b;
  _modelAmbient[3] = a;
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotExponent(float val) 
{ 
  _spotExponent = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotCutoff(float val) 
{ 
  _spotCutoff = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetConstantAttenuation(float val) 
{ 
  _constantAttenuation = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetLinearAttenuation(float val) 
{ 
  _linearAttenuation = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetQuadraticAttenuation(float val) 
{ 
  _quadraticAttenuation = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelLocalViewer(float val) 
{ 
  _modelLocalViewer = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelTwoSide(float val) 
{ 
  _modelTwoSide = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetPositionX(float val) 
{
  _position[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetPositionY(float val) 
{
  _position[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetPositionZ(float val) 
{ 
  _position[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetPositionW(float val) 
{ 
  _position[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotDirectionX(float val)  
{ 
  _spotDirection[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotDirectionY(float val) 
{
  _spotDirection[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpotDirectionZ(float val) 
{
  _spotDirection[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelAmbientR(float val) 
{
  _modelAmbient[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelAmbientG(float val) 
{
  _modelAmbient[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelAmbientB(float val) 
{
  _modelAmbient[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetModelAmbientA(float val) 
{
  _modelAmbient[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetAmbientR(float val) 
{
  _ambient[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetAmbientG(float val) 
{
  _ambient[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetAmbientB(float val) 
{
  _ambient[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetAmbientA(float val) 
{
  _ambient[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetDiffuseR(float val) 
{
  _diffuse[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetDiffuseG(float val) 
{
  _diffuse[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetDiffuseB(float val) 
{
  _diffuse[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetDiffuseA(float val) 
{
  _diffuse[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpecularR(float val) 
{
  _specular[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpecularG(float val) 
{
  _specular[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpecularB(float val) 
{
  _specular[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetSpecularA(float val) 
{
  _specular[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetBackgroundR(float val) 
{
  _background[0] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetBackgroundG(float val) 
{
  _background[1] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetBackgroundB(float val) 
{
  _background[2] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

void Light::SetBackgroundA(float val) 
{
  _background[3] = val; 
  _GLLRefreshFlag = true;
  RefreshGLList();
}

GLuint Light::GetGLList()
{
  if (_GLLRefreshFlag)
    {
      RefreshGLList();
      _GLLRefreshFlag = false;
    }

  return (_GLList);
}

void Light::RefreshGLList()
{
  if (!_GLList)
    {
      _GLList = glGenLists(1);
      if (!_GLList)
	AError("Light::GetGLList : not enough OGL memory.");
    }
  glNewList(_GLList, GL_COMPILE);
  glLightfv(GL_LIGHT0, GL_AMBIENT, _ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, _diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, _specular);
  glLightfv(GL_LIGHT0, GL_POSITION, _position);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, _spotDirection);
  glLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, &_spotExponent);
  glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &_spotCutoff);
  glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, &_constantAttenuation);
  glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, &_linearAttenuation);
  glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, &_quadraticAttenuation);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, _modelAmbient);
  glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, &_modelLocalViewer);
  glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, &_modelTwoSide);
  glClearColor(_background[0], _background[1], 
	       _background[2], _background[3]);
  glEndList();
}
