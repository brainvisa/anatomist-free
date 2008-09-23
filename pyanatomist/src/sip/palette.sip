namespace anatomist
{

class APalette : AimsData_RGBA
{
%TypeHeaderCode
#include <anatomist/color/palette.h>
%End

public:
  APalette(const std::string & = "", unsigned = 256,
	unsigned = 1, unsigned = 1, unsigned = 1);
  virtual ~APalette();

  std::string name() const;
  void setName( const std::string & );
};


class AObjectPalette
{
%TypeHeaderCode
#include <anatomist/color/objectPalette.h>
%End

public:
  AObjectPalette( rc_ptr_APalette );
  AObjectPalette(const anatomist::AObjectPalette &);
  virtual ~AObjectPalette();

  rc_ptr_APalette refPalette() const;
  rc_ptr_APalette refPalette2() const;
  void setRefPalette( rc_ptr_APalette pal );
  void setRefPalette2( rc_ptr_APalette pal );
  float min1() const;
  float max1() const;
  float min2() const;
  float max2() const;
  void create( unsigned dimx, unsigned dimy = 1, unsigned dimz = 1,
               unsigned dimt = 1 );
  virtual void fill();
};


class PaletteList
{
%TypeHeaderCode
#include <anatomist/color/paletteList.h>
%End

public:
  virtual ~PaletteList();

  unsigned size() const;
  virtual void load( const std::string &dirname, bool clr=false );
  virtual void save( const std::string &dirname, bool bin=false ) const;
  virtual void clear();
  virtual void erase( rc_ptr_APalette pal );
  virtual void push_back( rc_ptr_APalette pal );
  rc_ptr_APalette find( const std::string & );
  const list_rc_ptr_APalette & palettes() const;

private:
  PaletteList();
};

};

