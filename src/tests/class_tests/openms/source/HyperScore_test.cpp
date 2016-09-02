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
// $Maintainer: Timo Sachsenberg$
// $Authors: Timo Sachsenberg$
// --------------------------------------------------------------------------

#include <OpenMS/CONCEPT/ClassTest.h>

///////////////////////////
#include <OpenMS/ANALYSIS/RNPXL/HyperScore.h>
///////////////////////////

using namespace OpenMS;
using namespace std;

START_TEST(HyperScore, "$Id$")

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

HyperScore* ptr = 0;
HyperScore* null_ptr = 0;
START_SECTION(HyperScore())
{
  ptr = new HyperScore();
  TEST_NOT_EQUAL(ptr, null_ptr)
}
END_SECTION

START_SECTION(~HyperScore())
{
  delete ptr;
}
END_SECTION

START_SECTION((static double compute(double fragment_mass_tolerance, bool fragment_mass_tolerance_unit_ppm, const PeakSpectrum &exp_spectrum, const RichPeakSpectrum &theo_spectrum)))
{
  PeakSpectrum exp_spectrum;
  RichPeakSpectrum theo_spectrum;
  Peak1D p;
  p.setIntensity(1);
  RichPeak1D rp;
  rp.setIntensity(1);
  
  // full match, 5 identical masses, identical intensities (=1)
  for (Size i = 1; i != 6; ++i)
  {
    p.setMZ(i);
    rp.setMZ(i);
    rp.setMetaValue("IonName", String("y") + String(i));
    exp_spectrum.push_back(p);
    theo_spectrum.push_back(rp);
  }
  TEST_REAL_SIMILAR(HyperScore::compute(0.1, false, exp_spectrum, theo_spectrum), 7.39693);
  TEST_REAL_SIMILAR(HyperScore::compute(10, true, exp_spectrum, theo_spectrum), 7.39693);

  // full match, 10 identical masses, identical intensities (=1)
  for (Size i = 6; i <= 10; ++i)
  {
    p.setMZ(i);
    rp.setMZ(i);
    rp.setMetaValue("IonName", String("y") + String(i));
    exp_spectrum.push_back(p);
    theo_spectrum.push_back(rp);
  }
  TEST_REAL_SIMILAR(HyperScore::compute(0.1, false, exp_spectrum, theo_spectrum), 18.407);
  TEST_REAL_SIMILAR(HyperScore::compute(10, true, exp_spectrum, theo_spectrum), 18.407);

  exp_spectrum.clear(true);
  theo_spectrum.clear(true);

  // full match if ppm tolerance and partial match for Da tolerance
  for (Size i = 1; i <= 10; ++i)
  {
    double mz = pow(10.0, static_cast<int>(i));
    p.setMZ(mz);
    rp.setMZ(mz + 9 * 1e-6 * mz); // +9 ppm error
    rp.setMetaValue("IonName", String("b") + String(i));
    exp_spectrum.push_back(p);
    theo_spectrum.push_back(rp);
  }
  TEST_REAL_SIMILAR(HyperScore::compute(0.1, false, exp_spectrum, theo_spectrum), 5.5643482);
  TEST_REAL_SIMILAR(HyperScore::compute(10, true, exp_spectrum, theo_spectrum), 18.407);
}
END_SECTION

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST

