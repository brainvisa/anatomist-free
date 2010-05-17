/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
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
 * knowledge of the CeCILL-B license and that you accept its terms.
 */

#include "qAbout.h"
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qsound.h>
#include <qnamespace.h>
#include <qevent.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#ifndef _WIN32
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif
#include <sys/stat.h>
#ifdef _WIN32
#  include <io.h>
#else
#  ifdef SOMA_SOUND_ALSA
#    include <alsa/asoundlib.h>
#  endif
#  if defined(__linux)
#    ifdef SOMA_SOUND_OSS
#      include <sys/soundcard.h>
#    endif
#  elif defined(__sun)
#    include <sys/audioio.h>
#  endif
#  if defined( _WS_X11_ ) || defined( Q_WS_X11 )
#    include <X11/Xlib.h>
#  endif
#endif
#ifdef ANA_NO_SOUND
#define ABOUT_NO_SOUND
#endif

/* AudiQ classes sources - I directly include them so they don't appear
   externally
 */
#include "wavheader.cc"
#include "diffcode.cc"

using namespace std;
using namespace audiq;


namespace
{

  class QScrollingLabel : public QLabel
  {
  public:
    QScrollingLabel( QWidget* parent, const char* name = 0 );
    virtual ~QScrollingLabel();

    bool nextline;
    char *text;
    char *current;
    int  offset;
    int  speed;
    bool mustfill;

  protected:
    void paintEvent( QPaintEvent* );
    void showEvent( QShowEvent* );
    void resizeEvent( QResizeEvent* );
  };


  QScrollingLabel::QScrollingLabel( QWidget* parent, const char* name )
  : QLabel( parent, name ), nextline( false ), text( 0 ), current( 0 ),
  offset( 0 ), speed( 1 ), mustfill( true )
  {
  }


  QScrollingLabel::~QScrollingLabel()
  {
  }


  void QScrollingLabel::paintEvent( QPaintEvent* )
  {
    if( mustfill )
    {
      QPainter	paint( this );
      paint.setClipRect( 0, 0, width(), height() );
      paint.fillRect( 0, 0, width(), height(), QBrush( backgroundColor() ) );
      mustfill = false;
    }
    else if( text && nextline )
    {
      nextline = false;
      int	h = 16;
      int	n = speed;
      int	m = -2;

      int	x, y, w;
      x = 0;
      w = width();

      QPainter	paint( this );

      if( !current || current[0] == '\0' )
        current = text;
      char	*chr = strchr( current, '\n' );
      unsigned	l, of = 0;
      QFont	fnt = paint.font();

      while( current[of] == '/' )	// font code
      {
        ++of;
        switch( current[of] )
        {
          case 'b':	// bold
            fnt.setBold( true );
            ++of;
            break;
          case 'i':	// italic
            fnt.setItalic( true );
            ++of;
            break;
          case 'u':	// underline
            fnt.setUnderline( true );
            ++of;
            break;
          case 'h':	// height
            fnt.setPointSize( fnt.pointSize() + 2 );
            ++of;
            break;
          case 's':	// strikeout
            fnt.setStrikeOut( true );
            ++of;
            break;
          case 'f':	// fixed pitch
            fnt.setFixedPitch( true );
            ++of;
          default:	// not understood
            break;
        }
      }
      paint.setFont( fnt );
      QFontInfo	finf( fnt );	// I can't retreive the exact font height in
      h = int( fnt.pointSizeFloat() ) + 7;	// pixels !!
      //cout << "font size: " << h << endl;
      y = height()-h;

      if( chr )
        l = chr - current - of;
      else
        l = strlen( current + of );
      char	*line = new char[ l + 1 ];
      strncpy( line, current + of, l );
      line[ l ] = '\0';

      paint.setClipRect( x, y, w, h );
      paint.fillRect( x, height()-n, w, n,
                      QBrush( backgroundColor() ) );
      paint.drawText( x, height()+offset, w, h,
                      Qt::AlignHCenter | Qt::AlignBottom, line );
      delete[] line;
      paint.end();

      if( offset <= m-h )
      {
        offset = 0;
        if( chr )
          current += l+1+of;
        else
          current = text;
      }
      else
        offset -= n;
    }
  }


  void QScrollingLabel::showEvent( QShowEvent* )
  {
    mustfill = true;
  }


  void QScrollingLabel::resizeEvent( QResizeEvent* )
  {
    mustfill = true;
  }

}


struct QAbout::Private
{
  Private();

  QScrollingLabel	*edit;
#if( !defined( _WIN32 ) && !defined( ABOUT_NO_SOUND ) )
  pthread_t		musThrd;
#endif
  bool			threadRunning;
  QSound		*qsound;
  bool			diffcoded;
  string		musicfile;
  string		tempfile;
  bool                  useAlsa;
  bool                  useOSS;
  long                  soundBufferSize;
#ifdef SOMA_SOUND_ALSA
  snd_pcm_t             *alsaHandle;
#endif
#ifdef SOMA_SOUND_OSS
  int                   sndFD;
#endif
};


QAbout::Private::Private()
  : qsound( 0 ), diffcoded( false ), useAlsa( false ), useOSS( false ),
  soundBufferSize( 0 )
#ifdef SOMA_SOUND_ALSA
  , alsaHandle( 0 )
#endif
#ifdef SOMA_SOUND_OSS
  , sndFD( -1 )
#endif
{
}


QAbout::QAbout( QWidget* parent, const char* name ) 
  : QDialog( parent, name, true ), d( new Private )
{
  setCaption( tr( "About Anatomist" ) );

  QVBoxLayout	*lay1 = new QVBoxLayout( this, 10, -1, "lay1" );
  d->edit = new QScrollingLabel( this, "edit" );
  d->edit->setLineWidth( 2 );
  d->edit->setMidLineWidth( 2 );
  d->edit->setFrameStyle( QFrame::Sunken | QFrame::Panel );
  d->edit->setMinimumSize( 350, 250 );
  d->edit->setBackgroundColor( Qt::white );
#if QT_VERSION >= 0x040000
  d->edit->setAutoFillBackground( false );
  d->edit->setAttribute( Qt::WA_OpaquePaintEvent );
#endif

  QPushButton	*bok = new QPushButton( tr( "OK" ), this, "okbtn" );

  bok->setDefault( true );
  connect( bok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  bok->setFixedSize( bok->sizeHint() );

  lay1->addWidget( d->edit );
  lay1->addSpacing( 10 );
  lay1->addWidget( bok );
  lay1->addSpacing( 10 );

  resize( 400, 400 );

  struct stat	buf;
  string tname = scrollingMessageFileName().utf8().data();
  int		sres = stat( tname.c_str(), &buf );
  FILE	*	f = 0;

  if( sres == 0 )
    f = fopen( tname.c_str(), "r" );

  if( !sres && f )
    {
      d->edit->text = new char[ buf.st_size + 1 ];
      fread( d->edit->text, 1, buf.st_size, f );
      d->edit->text[ buf.st_size - 1 ] = '\0';
      fclose( f );
      d->edit->current = d->edit->text;

      QTimer	*tim = new QTimer( this, "timer" );
      connect( tim, SIGNAL( timeout() ), this, SLOT( nextLine() ) );
      tim->start( 25 );
    }
  else
    {
      QString	text = errorMessage();
      d->edit->setText( text );
    }

  d->musicfile = musicFileName().utf8().data();
  if( d->musicfile.substr( d->musicfile.length() - 4, 4 ) == ".adc" )
    d->diffcoded = true;
  d->tempfile = temporaryMusicFileName().utf8().data();
  cout << "musicFile:" << d->musicfile << endl;

#if defined( linux ) || defined( ABOUT_NO_SOUND )
  bool enableQSound = false;
#else
  bool enableQSound = true;
#endif
  if( enableQSound && !QSound::isAvailable() )
    enableQSound = false;

  if( enableQSound )
    {
      d->threadRunning = false;
      string	file = d->musicfile;
      if( d->diffcoded )
        {
          file = d->tempfile;
          DiffCode::uncompress( d->musicfile, d->tempfile );
        }
      d->qsound = new QSound( file.c_str(), this );
      d->qsound->play();
    }
  else
    {
#if defined( ABOUT_NO_SOUND ) || defined( _WIN32 )
      d->threadRunning = false;
#else
      d->threadRunning = true;
      pthread_create( &d->musThrd, 0, musicThread, this );
#endif
    }
}


QAbout::~QAbout()
{
#ifndef ABOUT_NO_SOUND
  if( d->threadRunning )
    {
      d->threadRunning =false;
#ifndef _WIN32
      void	*garbage;
      pthread_join( d->musThrd, &garbage );
#endif	// _WIN32
    }
#ifdef SOMA_SOUND_ALSA
    if( d->useAlsa )
    {
      snd_pcm_close( d->alsaHandle );
    }
#endif
#ifdef SOMA_SOUND_OSS
    if( d->useOSS && d->sndFD >= 0 )
    {
      //        *** DSP ***
#ifdef __linux
      ioctl( d->sndFD, SNDCTL_DSP_RESET, (char*)0 );
#endif
      ::close( d->sndFD );
      d->sndFD = -1;
    }
#endif
#endif // ABOUT_NO_SOUND

  if( d->qsound )
    {
      d->qsound->stop();
      delete d->qsound;
    }
  if( !d->tempfile.empty() )
    unlink( d->tempfile.c_str() );

  delete[] d->edit->text;
  d->edit->text = 0;

  delete d;
}


void QAbout::nextLine()
{
  if( d->edit->text )
  {
    int	n = d->edit->speed;
    d->edit->nextline = true;
    d->edit->scroll( 0, -n, QRect( 0, 0, d->edit->width(),
                     d->edit->height() ) );
  }
}


void * QAbout::musicThread( void* caller )
{
  ((QAbout *) caller)->music();
  return( 0 );
}


namespace
{

#if !defined( ABOUT_NO_SOUND ) && defined( SOMA_SOUND_ALSA )
  bool initSoundAlsa( QAbout::Private *d, const WavHeader & hdr )
  {
    const char    audiodev[] = "default";
    // Open the soundcard device.
    int err;

    d->useAlsa = false;
    d->alsaHandle = 0;
    snd_pcm_t *handle;

    if( ( err = snd_pcm_open( &handle, audiodev, SND_PCM_STREAM_PLAYBACK,
      0 ) ) < 0 )
    {
      cerr << "ALSA sound open error: " << snd_strerror(err) << endl;
      return false;
    }

    snd_pcm_hw_params_t *hw_params;
    if( ( err = snd_pcm_hw_params_malloc( &hw_params ) ) < 0 )
    {
      cerr << snd_strerror(err) << endl;
      snd_pcm_close(handle);
      return false;
    }

    // mono, 8 bits, 22 kHz
    snd_pcm_format_t format = SND_PCM_FORMAT_U8;
    if( hdr.sampleSize == 2 )
      format = SND_PCM_FORMAT_U16;
    unsigned freqDsp = hdr.rate;
    int channels = hdr.channels;
    if( ( err = snd_pcm_hw_params_any( handle, hw_params ) ) < 0 )
    {
      cerr << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    if( ( err = snd_pcm_hw_params_set_access( handle, hw_params,
          SND_PCM_ACCESS_RW_INTERLEAVED ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    if( ( err = snd_pcm_hw_params_set_format( handle, hw_params,
          format ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    if( ( err = snd_pcm_hw_params_set_rate_near( handle, hw_params, &freqDsp,
          0 ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    if( ( err = snd_pcm_hw_params_set_channels( handle, hw_params,
          channels ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    if( ( err = snd_pcm_hw_params( handle, hw_params ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }

    unsigned bufferTime = 25000;    // 1/40 second
    int dir = 1;

    err = snd_pcm_hw_params_set_buffer_time_near( handle, hw_params,
        &bufferTime, &dir);
    if (err < 0)
    {
      /*
      cerr << "Unable to set buffer time " << bufferTime << " for playback: "
        << snd_strerror(err) << endl;
      */
      snd_pcm_hw_params_free( hw_params );
      snd_pcm_close(handle);
      return false;
    }
    // cout << "buffer time: " << bufferTime << endl;

    snd_pcm_uframes_t bufsz = 0;
    err = snd_pcm_hw_params_get_buffer_size( hw_params, &bufsz );
    if (err < 0)
    {
      cerr << "Unable to get buffer size for playback: "
        << snd_strerror(err) << endl;
      // why doesn't this work ???
      bufsz = ((long long) bufferTime) * freqDsp / 1000000;
    }
    // cout << "obtained buffer size : " << bufsz << endl;
    snd_pcm_hw_params_free( hw_params );

    if( ( err = snd_pcm_prepare( handle ) ) < 0 )
    {
      cerr << "ALSA error: " << snd_strerror(err) << endl;
      snd_pcm_close(handle);
      return false;
    }

    d->alsaHandle = handle;
    d->soundBufferSize = bufsz;
    d->useAlsa = true;
    return true;
  }
#endif


#if !defined( ABOUT_NO_SOUND ) && defined( SOMA_SOUND_OSS )
  bool initSoundOSS( QAbout::Private *d, const WavHeader & hdr )
  {
#ifdef __linux	// specific...
    // mono, 8 bits, 22 kHz
    d->useOSS = false;
    int fd = d->sndFD;
    if( fd < 0 )
      return false;
    int arg = AFMT_U8;
    if( hdr.sampleSize == 2 )
      arg = AFMT_S16_LE;
    if( ioctl( d->sndFD, SNDCTL_DSP_SETFMT, (char*) &arg ) == -1 )
      {
        cerr << "Error while setting SNDCTL_DSP_SETFMT\n";
        //SOUND_PCM_WRITE_BITS\n";
        ::close( fd );
        return( false );
      }
    if( arg != hdr.sampleSize * 8 )
      {
        cerr << "Failed to set correct sample size.\n";
        ::close( fd );
        return( false );
      }

    arg = hdr.channels - 1;
    if( ioctl( fd, SNDCTL_DSP_STEREO, (char*)&arg ) == -1 )
      {
        cerr << "Error while setting SNDCTL_DSP_STEREO\n";
        ::close( fd );
        return( false );
      }
    if( arg != hdr.channels - 1 )
      {
        cerr << "This device doesn't support mono mode!?!\n";
        ::close( fd );
        return( false );
      }

    arg = hdr.rate;
    int	freqDsp = arg;
    if( ioctl( fd, SNDCTL_DSP_SPEED, (char*)&arg ) == -1 )
      {
        cerr << "Error while setting SNDCTL_DSP_SPEED to " << arg << "Hz\n";
        ::close( fd );
        return( false );
      }

    int	bufferSize = freqDsp / 40 * hdr.channels;	// 1/40 sec.
    // en exposant de 2
    unsigned	fragments = (unsigned) ::ceil( log( double(bufferSize) ) 
                                             / log( 2.0 ) + 0.5 );
    bufferSize = 1U << (fragments - 1);	// modif au plus proche
    //cout << "tagretted buffer size : " << bufferSize << endl;

    // 0xMMMMSSSS, MMMM = nb de fragments (2), SSSS = taille en exposant de 2
    fragments |= 0x20000;
    if( ioctl( fd, SNDCTL_DSP_SETFRAGMENT, (char*)&fragments ) == -1 )
      {
        cerr << "Couldn't set buffers size and numbers\n";
        ::close( fd );
        return( false );
      }
    bufferSize = 0x1 << ( ( fragments & 0xFFFF ) - 1 );
    //cout << "actual buffer size : " << bufferSize << endl;
    d->soundBufferSize = bufferSize;


#else	// not linux
#if defined( __sun )
    //	Why must we ask 4 kHz to obtain 8 ????????
    int	freqDsp = hdr.rate / 2;			// 8 kHz
    int	bufferSize = 512;			// buffer de 512 octets

    audio_info_t	auinf;

    AUDIO_INITINFO( &auinf );

    auinf.output_muted = 0;			// non mute
    auinf.play.sample_rate = freqDsp;
    auinf.play.channels = hdr.channels;
    auinf.play.precision = hdr.sampleSize * 8; // * channels ??
    auinf.play.encoding = AUDIO_ENCODING_LINEAR;	// 8 bits unsigned
    auinf.play.gain = AUDIO_MAX_GAIN / 2;		// volume max
    //auinf.play.port = AUDIO_SPEAKER;		// sortie haut-parleur interne
    // is it necessary to fill this ?
    //auinf.play.avail_ports = AUDIO_SPEAKER | AUDIO_HEADPHONE | AUDIO_LINEOUT;
    auinf.play.buffer_size = bufferSize;
    auinf.play.balance = AUDIO_MID_BALANCE;

    // write new values
    if( ioctl( fd, AUDIO_SETINFO, (char*)&auinf ) == -1 )
      {
        if( errno == EINVAL )
          cerr << "EINVAL\n";
        else if( errno == EBUSY )
          cerr << "EBUSY\n";
        cerr << "Failed to set audio params.\n";
        ::close( fd );
        return( false );
      }
    /*cout << "freq : " << auinf.play.sample_rate << ", precision : " 
      << auinf.play.precision << ", encoding : " << auinf.play.encoding 
      << ", buffsize : " << auinf.play.buffer_size << endl;*/
    d->soundBufferSize = bufferSize;

#else	// pas sun non plus: device non reconnu
    ::close( fd );
    return( false );
#endif
#endif
    d->useOSS = true;
    return( true );
  }
#endif	// sound compiled


#if !defined( ABOUT_NO_SOUND ) && ( defined( SOMA_SOUND_ALSA ) || defined( SOMA_SOUND_OSS ) )
  bool initSound( QAbout::Private *d, const WavHeader & hdr )
  {
    cout << "initSound\n";
    d->useAlsa = false;
    d->useOSS = false;
#ifdef SOMA_SOUND_ALSA
    if( initSoundAlsa( d, hdr ) )
      return true;
#endif
#ifdef SOMA_SOUND_OSS
    if( initSoundOSS( d, hdr ) )
      return true;
#endif
    return false;
  }
#endif

}


void QAbout::music()
{
#if !defined( ABOUT_NO_SOUND ) && ( defined( SOMA_SOUND_OSS ) || defined( SOMA_SOUND_ALSA ) )

#if ( defined( _WS_X11_ ) || defined( Q_WS_X11 ) )

  /*    ensure program and display are on the same machine
        (to avoid sound being heard on a remote machine)
  */
  char	*display = DisplayString( x11Display() );
  if( !display )
    return;	// can't get display name: no sound

  if( display[0] != ':' )	// not default display
  {
    char	host[50];
    if( gethostname( host, 50 ) < 0 )
      return;	// can't figure out hostname: no sound

    string disp = display;
    string::size_type pos = disp.find( ':' );
    if( pos == string::npos )
      return;	// don't understand display string
    disp.erase( pos, disp.size() - pos );	// keep only name
    if( disp != host )
      return;	// display and process machines are different: no sound
  }
#endif

  string        name = d->musicfile;

#ifdef SOMA_SOUND_OSS
#ifdef __linux
  const char	audiodev[] = "/dev/dsp";
#else
  const char	audiodev[] = "/dev/audio";
#endif

  // start playing sound if sound file and audio device are both OK
  struct stat	buf;

  if( !stat( audiodev, &buf ) && !stat( name.c_str(), &buf ) )
  {
    if( d->sndFD < 0 )
      d->sndFD = ::open( audiodev, O_WRONLY ); // in Qt4, QDialog has an open() method
  }
#endif // SOMA_SOUND_OSS

  ifstream		sndstr( name.c_str(), ios::in | ios::binary );
  sndstr.unsetf( ios::skipws );

  DiffCode::CompressInfo	info;
  WavHeader			& hdr = info.hdr;
  DiffCode::CompressedPos	pos;

  try
  {
    if( d->diffcoded )
    {
      info.read( sndstr, name );
      pos = DiffCode::CompressedPos( 0, hdr.channels );
    }
    else
      hdr.read( sndstr, name );
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    d->threadRunning = false;
    return;
  }

  if( !initSound( d, hdr ) )
  {
    d->threadRunning = false;
    return;
  }
  // cout << "initSound ok, alsa: " << d->useAlsa << ", oss: " << d->useOSS
  //   << endl;

  vector<char>	mbuf( d->soundBufferSize * hdr.sampleSize * hdr.channels );
  int n, sz = hdr.size;
#ifdef SOMA_SOUND_ALSA
  int done;
  snd_pcm_sframes_t frames = -1;
#endif
  // cout << "buffer size: " << mbuf.size() << endl;
  // cout << "sampleSize: " << hdr.sampleSize << ", channels: " << hdr.channels
  //   << ", soundBufferSize: " << d->soundBufferSize << endl;

  while( sz > 0 && d->threadRunning )
  {
    n = d->soundBufferSize;
    if( sz < n )
      n = sz;
    sz -= n;
    if( d->diffcoded )
      pos = DiffCode::uncompress( sndstr, info, &mbuf[0], pos, n );
    else
      sndstr.read( &mbuf[0], n );

    if( d->useAlsa )
    {
#ifdef SOMA_SOUND_ALSA
      // cout << "n: " << n << endl;
      frames = -1;
      done = 0;
      while( n > 0 )
      {
        // cout << "n: " << n << endl;
        frames = snd_pcm_writei( d->alsaHandle, &mbuf[done], n );
        // cout << "frames: " << frames << endl;
        if( frames < 0 )
        {
          // cout << "snd_pcm_writei < 0: " << snd_strerror(frames) << endl;
          frames = snd_pcm_recover( d->alsaHandle, frames, 1 );
        }
        else
        {
          done += frames;
          n -= frames;
        }
      }
#endif
    }
    else if( d->useOSS )
    {
#ifdef SOMA_SOUND_OSS
    //	*** Solaris: flush ***
#if defined( __sun )
    ioctl( d->sndFD, AUDIO_DRAIN, (char*)0 );
#endif
    write( d->sndFD, &mbuf[0], n * hdr.sampleSize * hdr.channels );
#endif
    }
  }

  sndstr.close();

  d->threadRunning = false;

#endif	// ABOUT_NO_SOUND
}


void QAbout::keyPressEvent( QKeyEvent* kev )
{
  if( kev->ascii() == '+' )
    {
      ++d->edit->speed;
      kev->accept();
    }
  else if( kev->ascii() == '-' )
    {
      if( d->edit->speed > 1 )
        --d->edit->speed;
      kev->accept();
    }
  else if( kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return )
    accept();
  else if( kev->ascii() == ' ' )
    {
      if( d->threadRunning )
	{
	  d->threadRunning = false;
#if ( defined( __linux ) && !defined( ABOUT_NO_SOUND ) )
	  void	*garbage;
	  pthread_join( d->musThrd, &garbage );
#ifdef SOMA_SOUND_OSS
          if( d->useOSS && d->sndFD >= 0 )
            ioctl( d->sndFD, SNDCTL_DSP_RESET, (char*)0 );
#endif
#endif	// ABOUT_NO_SOUND
	}
      else
	{
	  d->threadRunning = true;
#if ( !defined( _WIN32 ) && !defined( ABOUT_NO_SOUND ) )
	  pthread_create( &d->musThrd, 0, musicThread, this );
#endif
	}
    }
  else
    kev->ignore();
}

