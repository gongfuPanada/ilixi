/*
 Copyright 2010, 2011 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 This file is part of ilixi.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with ilixi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types/Video.h"
#include "core/Window.h"
#include "core/Logger.h"
#include <sstream>
#include <iomanip>

using namespace ilixi;

static void
videoCB(void *cdata)
{
  Video* v = (Video*) cdata;
  v->frameUpdated(v->frame());
}

Video::Video(const std::string& path) :
  _provider(NULL), _frame(NULL)
{
  load(path);
}

Video::~Video()
{
  if (_provider)
    _provider->Release(_provider);
  if (_frame)
    _frame->Release(_frame);
}

double
Video::position() const
{
  double sec = 0;
  if (_provider)
    _provider->GetPos(_provider, &sec);
  return sec;
}

double
Video::length() const
{
  double length = 0;
  if (_provider)
    _provider->GetLength(_provider, &length);
  return length;
}

double
Video::speed() const
{
  double speed = 0;
  if (_provider)
    _provider->GetSpeed(_provider, &speed);
  return speed;
}

float
Video::volume() const
{
  float volume = 0;
  if (_provider)
    _provider->GetVolume(_provider, &volume);
  return volume;
}

DFBVideoProviderStatus
Video::status() const
{
  DFBVideoProviderStatus stat = DVSTATE_UNKNOWN;
  if (_provider)
    _provider->GetStatus(_provider, &stat);
  return stat;
}

const char*
Video::encodingVideo() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO)
      && *_streamDesc.video.encoding)
    return _streamDesc.video.encoding;
  return "Unknown";
}

int
Video::width() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO))
    return _surfaceDesc.width;
  return 0;
}

int
Video::height() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO))
    return _surfaceDesc.height;
  return 0;
}

double
Video::aspect() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO))
    return _streamDesc.video.aspect;
  return 0;
}

double
Video::framerate() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO))
    return _streamDesc.video.framerate;
  return 0;
}

int
Video::bitrateVideo() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_VIDEO))
    return _streamDesc.video.bitrate / 1000;
  return 0;
}

const char*
Video::encodingAudio() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_AUDIO)
      && *_streamDesc.audio.encoding)
    return _streamDesc.audio.encoding;
  return "Unknown";
}

int
Video::samplerate() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_AUDIO))
    return _streamDesc.audio.samplerate / 1000;
  return 0;
}

int
Video::channels() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_AUDIO))
    return _streamDesc.audio.channels;
  return 0;
}

int
Video::bitrateAudio() const
{
  if (_provider && (_streamDesc.caps & DVSCAPS_AUDIO))
    return _streamDesc.audio.bitrate / 1000;
  return 0;
}

const char*
Video::title() const
{
  if (_provider)
    return _streamDesc.title;
  return "N/A";
}

const char*
Video::author() const
{
  if (_provider)
    return _streamDesc.author;
  return "N/A";
}

const char*
Video::album() const
{
  if (_provider)
    return _streamDesc.album;
  return "N/A";
}

int
Video::year() const
{
  if (_provider)
    return _streamDesc.year;
  return 0;
}

const char*
Video::genre() const
{
  if (_provider)
    return _streamDesc.genre;
  return "N/A";
}

const char*
Video::comment() const
{
  if (_provider)
    return _streamDesc.comment;
  return "N/A";
}

IDirectFBSurface*
Video::frame() const
{
  return _frame;
}

bool
Video::load(const std::string& path)
{

  int result = access(path.c_str(), F_OK);
  if (result != 0)
    {
      ILOG_ERROR("File (%s) is not accessible!", path.c_str())
        ;
      return false;
    }

  if (_provider)
    _provider->Release(_provider);

  int ret = Window::DFBInterface()->CreateVideoProvider(Window::DFBInterface(),
      path.c_str(), &_provider);
  if (ret != DFB_OK)
    {
      _provider = NULL;
      ILOG_ERROR("Unable to create DFBVideoProvider [%d]!", ret)
        ;
      return false;
    }
  else
    {
      _path = path;
      _provider->GetStreamDescription(_provider, &_streamDesc);

      if (_streamDesc.caps & DVSCAPS_VIDEO)
        {
          _provider->GetSurfaceDescription(_provider, &_surfaceDesc);
          Window::DFBInterface()->CreateSurface(Window::DFBInterface(),
              &_surfaceDesc, &_frame);
        }

      _provider->GetCapabilities(_provider, &_providerCaps);
      return true;
    }
}

void
Video::play()
{
  if (_provider)
    _provider->PlayTo(_provider, _frame, NULL, videoCB, (void*) this);
}

void
Video::pause()
{
  if (_provider && (_providerCaps & DVCAPS_SPEED))
    _provider->SetSpeed(_provider, 0);
  else
    ILOG_ERROR("DFBVideoProvider: Pause is not supported!")
      ;
}

void
Video::seek(double secs)
{
  if (_provider && (_providerCaps & DVCAPS_SEEK))
    _provider->SeekTo(_provider, secs);
  else
    ILOG_ERROR("DFBVideoProvider: Seek is not supported!")
      ;
}

void
Video::stop()
{
  if (_provider)
    _provider->Stop(_provider);
}

void
Video::setSpeed(double speed)
{
  if (_provider && (_providerCaps & DVCAPS_SPEED))
    _provider->SetSpeed(_provider, speed);
  else
    ILOG_ERROR("DFBVideoProvider: SetSpeed is not supported!")
      ;
}

void
Video::setVolume(float level)
{
  if (_provider && (_providerCaps & DVCAPS_VOLUME))
    _provider->SetVolume(_provider, level);
  else
    ILOG_ERROR("DFBVideoProvider: setVolume is not supported!")
      ;
}

std::string
Video::toString()
{
  if (_path == "")
    return "N/A";
  std::stringstream ss;
  ss << "File: " << _path << "\n";
  if (_streamDesc.caps & DVSCAPS_VIDEO)
    {
      ss << "Video\nEncoding: "
          << (*_streamDesc.video.encoding ? _streamDesc.video.encoding
              : "Unknown") << "\nSize: " << _surfaceDesc.width << "x"
          << _surfaceDesc.height << "\nRatio: " << std::setprecision(3)
          << _streamDesc.video.aspect << "\nFramerate: "
          << _streamDesc.video.framerate << " fps\nBitrate: "
          << _streamDesc.video.bitrate / 1000 << " Kbits/s\n";
    }
  if (_streamDesc.caps & DVSCAPS_AUDIO)
    {
      ss << "\nAudio\nEncoding: "
          << (*_streamDesc.audio.encoding ? _streamDesc.audio.encoding
              : "Unknown") << "\nSamplerate: " << _streamDesc.audio.samplerate
          / 1000 << " Khz\nChannels: " << _streamDesc.audio.channels
          << "\nBitrate: " << _streamDesc.audio.bitrate / 1000 << " Kbits/s\n";
    }

  ss << "\nTitle: " << _streamDesc.title << "\nAuthor: " << _streamDesc.author
      << "\nAlbum: " << _streamDesc.album << "\nYear: " << _streamDesc.year
      << "\nGenre: " << _streamDesc.genre << "\nComment: "
      << _streamDesc.comment << "\n";

  return ss.str();
}
