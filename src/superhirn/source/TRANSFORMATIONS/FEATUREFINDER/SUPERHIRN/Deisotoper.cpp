// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2016.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Lukas Mueller, Markus Mueller $
// --------------------------------------------------------------------------
//
/*
 *  Deisotoper.cpp
 *  PeakDetection
 *
 *  Created by Markus Mueller on 10/19/06.
 *
 *  Ported to OpenMS by Florian Zeller, florian.zeller@bsse.ethz.ch
 *  December 2010
 *
 */

#include <list>
#include <iostream>
#include <map>

#include <OpenMS/TRANSFORMATIONS/FEATUREFINDER/SUPERHIRN/Deisotoper.h>
#include <OpenMS/TRANSFORMATIONS/FEATUREFINDER/SUPERHIRN/CentroidPeak.h>
#include <OpenMS/TRANSFORMATIONS/FEATUREFINDER/SUPERHIRN/CentroidData.h>
#include <OpenMS/TRANSFORMATIONS/FEATUREFINDER/SUPERHIRN/IsotopicDist.h>
#include <OpenMS/TRANSFORMATIONS/FEATUREFINDER/SUPERHIRN/SuperHirnParameters.h>

namespace OpenMS
{

  using namespace std;

// int Deisotoper::sfMinCharge = 1; // minimum charge considered in peak detection
// int Deisotoper::sfMaxCharge = 5; // maximum charge considered in peak detection

// Default constructor
  Deisotoper::Deisotoper()
  {
    fMinPeakGroupSize = 0;
    fTheta = 0.0;
    fScanNumber = -1;
  }

// Constructor. Takes centroide values and deisotopes them
  Deisotoper::Deisotoper(CentroidData & pCentroidData)  // Data objects containing centroid values
  {
    go(pCentroidData);
  }

// destructor
  Deisotoper::~Deisotoper()
  {
    fDeconvPeaks.clear();
  }

// Operators

// Writes data to out stream using the << operator
  ostream & operator<<(ostream & pOut, // output stream
                       Deisotoper & pDeisotoper) //
  {
    list<DeconvPeak> p;
    list<DeconvPeak>::iterator pi;

    p = pDeisotoper.getDeconvPeaks();

    for (pi = p.begin(); pi != p.end(); ++pi)
    {
      if (pDeisotoper.getShortReportFlag())
      {
        pOut << *((CentroidPeak *) &(*pi)) << endl;
      }
      else
      {
        pOut << *pi << " " << pDeisotoper.getScanNumber() << endl;
      }
    }

    return pOut;
  }

// Takes centroide values and deisotopes them
//** reads through the spectrum peak group by peak group (close enough peaks), **
//** looking for a set of peaks that match empirically tested isotopic patterns, **
//** The matching set of peaks is then stripped down to the monoisotopic
  void Deisotoper::go(CentroidData & pCentroidData)  // Data objects containing centroid values
  {
    int cnt, charge;
    double alpha;
    bool matched;
    
    //** pointer to peaks in pCentroidData **
    list<CentroidPeak> centroidPeaks;
    
    //** group start, group end, and group iterator **
    list<CentroidPeak>::iterator start, end, pi;
    
    // ** what is this?? **
    list<list<CentroidPeak>::iterator> matchedPeaks;

    pCentroidData.get(centroidPeaks);

    //** hello hardcoding (config param not shown to user)**
    fMinPeakGroupSize = 2;

    //** set the minimum intensity (depending on config params)**
    if (SuperHirnParameters::instance()->getMinIntensity() < SuperHirnParameters::instance()->getIntensityFloor())
    {
      pCentroidData.setNoise(30.0);       // set noise level at 30 prcentile
      fTheta = pCentroidData.getNoise();
    }
    else
    {
      fTheta = SuperHirnParameters::instance()->getMinIntensity();
    }

    //** sets peak iterator to beginning of peak vector **
    pCentroidData.resetPeakGroupIter();
    
    //** getNextPeakGroup returns a run of peaks (by bounding iterators start and end) **
    //** with max separation 1 + epsilon (config parameter) **
    while (pCentroidData.getNextPeakGroup(start, end)) // isotopic patterns are withing the same peak group
    {
      //** count the number of peaks in the peak group (in the iterator way!) **
      for (cnt = 0, pi = start; pi != end; ++pi, ++cnt)
      {
      }

      //** ignore single peaks (fMinPeakGroupSize = 2) **
      if (cnt >= fMinPeakGroupSize) // Discard peak groups with only one peak
      {
          
        //** iterate through peaks in peak group (pi) looking for monoisotopic peak (if any)**
        for (pi = start; pi != end; ++pi, --cnt)
        {
          //** ignore peaks below the minimum intensity **
          if (pi->getIntensity() < fTheta || cnt < fMinPeakGroupSize)
            continue;             // skip small peaks
          /*
           if( CentroidData::MonoIsoDebugging ){
           if( ( CentroidData::DebugMonoIsoMassMin <= pi->getMass()) && ( CentroidData::DebugMonoIsoMassMax >= pi->getMass()) ){
           cout<<"To deisotope: "<<pi->getMass()<<endl;
           }
           }
           */
           
          // ** iterate through all possible charge states (backwards) (config params) **
          for (charge = SuperHirnParameters::instance()->getMaxFeatureChrg(); charge >= SuperHirnParameters::instance()->getMinFeatureChrg(); --charge)
          {
            
            // get peak that match isotopic pattern of charge
            //** returns a set of peaks that match empirically tested mass/intensity readings **
            //** within the peak group from current peak to end of group **
            matched = IsotopicDist::getMatchingPeaks(pi, end, charge, alpha, fTheta, matchedPeaks);  
            if (matched && pi->getIntensity() >= fTheta) // subtract isotopic match from peaks if match is significant
            {
              /*
               if( CentroidData::MonoIsoDebugging ){
               if( ( CentroidData::DebugMonoIsoMassMin <= pi->getMass()) && ( CentroidData::DebugMonoIsoMassMax >= pi->getMass()) ){
               cout<<"matched to +"<<charge<<endl;
               }
               }
               */
    
              //** current peak checked out as monoisotopic peak, convert to deconvpeak **
              DeconvPeak mono(pi->getMass(), 0.0, charge, 0, 0.0, 0.0);
              if (!pi->getExtraPeakInfo().empty())
              {
                mono.setExtraPeakInfo(pi->getExtraPeakInfo());
              }

              //** calculate attributes of monoisotopic peak from matched peaks **
              IsotopicDist::subtractMatchingPeaks(matchedPeaks, charge, alpha, mono);
              fDeconvPeaks.push_back(mono);

              /*
               if( CentroidData::MonoIsoDebugging ){
               if( ( CentroidData::DebugMonoIsoMassMin <= pi->getMass()) && ( CentroidData::DebugMonoIsoMassMax >= pi->getMass()) ){
               mono.show_info();
               }
               }
               */
            }
            matchedPeaks.clear();
          }
        }
      }
    }
  }

// removes spooky or very small monoisotopic peaks
//** removes monoisotopic peaks that are under half the maximum intensity of their "cluster" **
//** seems quite arbitrary **
  void Deisotoper::cleanDeconvPeaks()
  {
    list<DeconvPeak>::iterator pi, beg, end, most_intense;

    //** iterate through monoisotopic peaks as clusters (note how pi is manipulated heavily inside loop **
    for (pi = fDeconvPeaks.begin(); pi != fDeconvPeaks.end(); ++pi)
    {
      double tol, mass;
      beg = pi;
      mass = pi->getMass();
      most_intense = pi;
      tol = SuperHirnParameters::instance()->getMassTolPpm() * mass / 1.0e6
            + SuperHirnParameters::instance()->getMassTolDa();
      ++pi;
      
      //** iterate through following peaks within tolerated range (cluster), finding most intense peak **
      for (; pi != fDeconvPeaks.end(); ++pi) // cluster peaks and define max intensity within cluster
      {
        if (pi->getMass() > mass + 2.0 * tol)
        {
          break;
        }

        if (most_intense->getIntensity() < pi->getIntensity())
        {
          most_intense = pi;           // store most intense peak
        }
      }
      end = pi;

      //** iterate through clustered peaks, eliminating those less than half the intensity of the max intensity cluster **
      for (pi = beg; pi != fDeconvPeaks.end() && pi != end; ++pi) // remove all 'very' small peak within cluster
      { // cout << "remove: " << pi->getMass() << " " << pi->getIntensity() << " " << pi->getCharge() << " | " << most_intense->getMass() << " " << most_intense->getIntensity() << endl;
        if (most_intense->getIntensity() > 2.0 * pi->getIntensity())
        {
// cout << "remove: " << pi->getMass() << " " << pi->getIntensity() << endl;
          pi = fDeconvPeaks.erase(pi);
          if (pi != fDeconvPeaks.begin())           // FLO: Fix windows error (crash "could not decrement")
            --pi;
        }
      }

      if (pi != fDeconvPeaks.end())
      {
        --pi;
      }
      else
      {
        break;
      }
    }
  }

}
