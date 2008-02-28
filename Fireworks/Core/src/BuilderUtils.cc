#include "Fireworks/Core/interface/BuilderUtils.h"
#include <math.h>
#include "TEveTrack.h"
#include "TEveTrackPropagator.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "TEveGeoShapeExtract.h"
#include "TGeoBBox.h"
#include "TColor.h"
#include "TROOT.h"
#include "TEveTrans.h"
#include "TEveGeoNode.h"

std::pair<double,double> fw::getPhiRange( const std::vector<double>& phis, double phi )
{
   double min =  100;
   double max = -100;

   for ( std::vector<double>::const_iterator i = phis.begin(); 
	 i != phis.end(); ++i ) 
     {
	double aphi = *i;
	// make phi continuous around jet phi
	if ( aphi - phi > M_PI ) aphi -= 2*M_PI;
	if ( phi - aphi > M_PI ) aphi += 2*M_PI;
	if ( aphi > max ) max = aphi;
	if ( aphi < min ) min = aphi;
     }
   
   if ( min > max ) return std::pair<double,double>(0,0);
   
   return std::pair<double,double>(min,max);
}

TEveTrack* fw::getEveTrack( const reco::Track& track,
			    double max_r /* = 120 */,
			    double max_z /* = 300 */,
			    double magnetic_field /* = 4 */ )
{
   TEveTrackPropagator *propagator = new TEveTrackPropagator();
   propagator->SetMagField( - magnetic_field );
   propagator->SetMaxR( max_r );
   propagator->SetMaxZ( max_z );

   TEveRecTrack t;
   t.fBeta = 1.;
   t.fP = TEveVector( track.px(), track.py(), track.pz() );
   t.fV = TEveVector( track.vx(), track.vy(), track.vz() );
   t.fSign = track.charge();
   TEveTrack* trk = new TEveTrack(&t, propagator);
   trk->MakeTrack();
   return trk;
}

std::string fw::NamedCounter::str() const
{
   std::stringstream s;
   s << m_name << m_index;
   return s.str();
}

TEveGeoShapeExtract* fw::getShapeExtract( const char* name,
					  TGeoBBox* shape,
					  Color_t color )
{
   TEveGeoShapeExtract* extract = new TEveGeoShapeExtract(name);
   TColor* c = gROOT->GetColor(color);
   Float_t rgba[4] = { 1, 0, 0, 1 };
   if (c) {
      rgba[0] = c->GetRed();
      rgba[1] = c->GetGreen();
      rgba[2] = c->GetBlue();
   }
   extract->SetRGBA(rgba);
   extract->SetRnrSelf(true);
   extract->SetRnrElements(true);
   extract->SetShape(shape);
   return extract;
}

void fw::addRhoZEnergyProjection( TEveElement* container,
			      double r_ecal, double z_ecal, 
			      double theta_min, double theta_max, 
			      double phi,
			      Color_t color)
{
   
   double z1 = r_ecal/tan(theta_min);
   if ( z1 > z_ecal ) z1 = z_ecal;
   if ( z1 < -z_ecal ) z1 = -z_ecal;
   double z2 = r_ecal/tan(theta_max);
   if ( z2 > z_ecal ) z2 = z_ecal;
   if ( z2 < -z_ecal ) z2 = -z_ecal;
   double r1 = z_ecal*fabs(tan(theta_min));
   if ( r1 > r_ecal ) r1 = r_ecal;
   if ( phi < 0 ) r1 = -r1;
   double r2 = z_ecal*fabs(tan(theta_max));
   if ( r2 > r_ecal ) r2 = r_ecal;
   if ( phi < 0 ) r2 = -r2;
   TColor* c = gROOT->GetColor( color );
   Float_t rgba[4] = { 1, 0, 0, 1 };
   if (c) {
      rgba[0] = c->GetRed();
      rgba[1] = c->GetGreen();
      rgba[2] = c->GetBlue();
   }
   
   if ( fabs(r2 - r1) > 1 ) {
      TGeoBBox *sc_box = new TGeoBBox(0., fabs(r2-r1)/2, 1);
      TEveTrans t;
      t(1,4) = 0; 
      t(2,4) = (r2+r1)/2;
      t(3,4) = fabs(z2)>fabs(z1) ? z2 : z1;
      TEveGeoShapeExtract *extract = new TEveGeoShapeExtract("r-segment");
      extract->SetTrans(t.Array());
      extract->SetRGBA(rgba);
      extract->SetRnrSelf(true);
      extract->SetRnrElements(true);
      extract->SetShape(sc_box);
      TEveElement* element = TEveGeoShape::ImportShapeExtract(extract, 0);
      element->SetPickable(kTRUE);
      container->AddElement(element);
   }
   if ( fabs(z2 - z1) > 1 ) {
      TGeoBBox *sc_box = new TGeoBBox(0., 1, (z2-z1)/2);
      TEveTrans t;
      t(1,4) = 0; 
      t(2,4) = fabs(r2)>fabs(r1) ? r2 : r1;
      t(3,4) = (z2+z1)/2;
      TEveGeoShapeExtract *extract = new TEveGeoShapeExtract("z-segment");
      extract->SetTrans(t.Array());
      extract->SetRGBA(rgba);
      extract->SetRnrSelf(true);
      extract->SetRnrElements(true);
      extract->SetShape(sc_box);
      TEveElement* element = TEveGeoShape::ImportShapeExtract(extract, 0);
      element->SetPickable(kTRUE);
      container->AddElement(element);
   }
}
