///
/// \class l1t::CaloStage1JetAlgorithmImpHI
///
///
/// \author: R. Alex Barbieri MIT
///

// This example implemenents algorithm version 1 and 2.

#include "CaloStage1JetAlgorithmImp.h"

// Taken from UCT code. Might not be appropriate. Refers to legacy L1 objects.
#include "DataFormats/L1CaloTrigger/interface/L1CaloCollections.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegion.h"
#include "DataFormats/L1CaloTrigger/interface/L1CaloRegionDetId.h"


using namespace std;
using namespace l1t;

CaloStage1JetAlgorithmImpHI::CaloStage1JetAlgorithmImpHI(/*const CaloParams & dbPars*/)/* : db(dbPars)*/ {}
CaloStage1JetAlgorithmImpHI::~CaloStage1JetAlgorithmImpHI(){};

// needs to come from db eventually
double regionLSB_ = 0.5;

double regionPhysicalEt(const L1CaloRegion& cand) {
  return regionLSB_*cand.et();
}

void puSubtraction(const std::vector<l1t::CaloRegion> & regions, vector<double> puLevelHI );
void makeJets(const std::vector<l1t::CaloRegion> & regions, vector<double> puLevelHI,
	      std::vector<l1t::Jet> & jets);


void CaloStage1JetAlgorithmImpHI::processEvent(const std::vector<l1t::CaloRegion> & regions,
					       std::vector<l1t::Jet> & jets){
  vector<double> puLevelHI;
  puLevelHI.resize(L1CaloRegionDetId::N_ETA);
  for(unsigned i = 0; i < L1CaloRegionDetId::N_ETA; ++i)
    puLevelHI[i] = 0;


  puSubtraction(regions, puLevelHI);
  makeJets(regions, puLevelHI, jets);

  // std::vector<l1t::CaloRegion>::const_iterator incell;
  // for (incell = regions.begin(); incell != regions.end(); ++incell){
  //   //do nothing for now
  // }

}

// NB PU is not in the physical scale!!  Needs to be multiplied by regionLSB
void puSubtraction(const std::vector<l1t::CaloRegion> & regions, vector<double> puLevelHI )
{
  double r_puLevelHI[L1CaloRegionDetId::N_ETA];
  int etaCount[L1CaloRegionDetId::N_ETA];
  for(unsigned i = 0; i < L1CaloRegionDetId::N_ETA; ++i)
  {
    puLevelHI[i] = 0;
    r_puLevelHI[i] = 0.0;
    etaCount[i] = 0;
  }

  for(vector<double>::const_iterator region = regions->begin(); region != regions->end(); region++){
    r_puLevelHI[region->gctEta()] += region->et();
    etaCount[region->gctEta()]++;
  }

  for(unsigned i = 0; i < L1CaloRegionDetId::N_ETA; ++i)
  {
    puLevelHI[i] = floor(r_puLevelHI[i]/etaCount[i] + 0.5);
  }
}

void makeJets(const std::vector<l1t::CaloRegion> & regions, vector<double> puLevelHI,
	      std::vector<l1t::Jet> & jets)
{
  for(vector<double>::const_iterator region = regions->begin(); region != regions->end(); region++) {
    double regionET = regionPhysicalEt(*region);
    regionET = std::max(0.,regionET -
			(puLevelHI[region->gctEta()]*regionLSB_));
    double neighborN_et = 0;
    double neighborS_et = 0;
    double neighborE_et = 0;
    double neighborW_et = 0;
    double neighborNE_et = 0;
    double neighborSW_et = 0;
    double neighborNW_et = 0;
    double neighborSE_et = 0;
    unsigned int nNeighbors = 0;
    for(vector<double>::const_iterator neighbor = regions->begin(); neighbor != regions->end(); neighbor++) {
      double neighborET = regionPhysicalEt(*neighbor);
      if(deltaGctPhi(*region, *neighbor) == 1 &&
	 (region->gctEta()    ) == neighbor->gctEta()) {
	neighborN_et = std::max(0.,neighborET -
				(puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == -1 &&
	      (region->gctEta()    ) == neighbor->gctEta()) {
	neighborS_et = std::max(0.,neighborET -
				(puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == 0 &&
	      (region->gctEta() + 1) == neighbor->gctEta()) {
	neighborE_et = std::max(0.,neighborET -
				(puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == 0 &&
	      (region->gctEta() - 1) == neighbor->gctEta()) {
	neighborW_et = std::max(0.,neighborET -
				(puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == 1 &&
	      (region->gctEta() + 1) == neighbor->gctEta()) {
	neighborNE_et = std::max(0.,neighborET -
				 (puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == -1 &&
	      (region->gctEta() - 1) == neighbor->gctEta()) {
	neighborSW_et = std::max(0.,neighborET -
				 (puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == 1 &&
	      (region->gctEta() - 1) == neighbor->gctEta()) {
	neighborNW_et = std::max(0.,neighborET -
				 (puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
      else if(deltaGctPhi(*region, *neighbor) == -1 &&
	      (region->gctEta() + 1) == neighbor->gctEta()) {
	neighborSE_et = std::max(0.,neighborET -
				 (puLevelHI[neighbor->gctEta()]*regionLSB_));
	nNeighbors++;
	continue;
      }
    }
    if(regionET > neighborN_et &&
       regionET > neighborNW_et &&
       regionET > neighborW_et &&
       regionET > neighborSW_et &&
       regionET >= neighborNE_et &&
       regionET >= neighborE_et &&
       regionET >= neighborSE_et &&
       regionET >= neighborS_et) {
      unsigned int jetET = regionET +
	neighborN_et + neighborS_et + neighborE_et + neighborW_et +
	neighborNE_et + neighborSW_et + neighborSE_et + neighborNW_et;
      /*
	int jetPhi = region->gctPhi() * 4 +
	( - 2 * (neighborS_et + neighborSE_et + neighborSW_et)
	+ 2 * (neighborN_et + neighborNE_et + neighborNW_et) ) / jetET;
	if(jetPhi < 0) {

	}
	else if(jetPhi >= ((int) N_JET_PHI)) {
	jetPhi -= N_JET_PHI;
	}
	int jetEta = region->gctEta() * 4 +
	( - 2 * (neighborW_et + neighborNW_et + neighborSW_et)
	+ 2 * (neighborE_et + neighborNE_et + neighborSE_et) ) / jetET;
	if(jetEta < 0) jetEta = 0;
	if(jetEta >= ((int) N_JET_ETA)) jetEta = N_JET_ETA - 1;
      */
      // Temporarily use the region granularity -- we will try to improve as above when code is debugged
      int jetPhi = region->gctPhi();
      int jetEta = region->gctEta();

      bool neighborCheck = (nNeighbors == 8);
      // On the eta edge we only expect 5 neighbors
      if (!neighborCheck && (jetEta == 0 || jetEta == 21) && nNeighbors == 5)
	neighborCheck = true;

      if (!neighborCheck) {
	std::cout << "phi: " << jetPhi << " eta: " << jetEta << " n: " << nNeighbors << std::endl;
	assert(false);
      }
      LorentzVector *jetLorentz = new LorentzVector();
      l1t::Jet theJet(jetLorentz, jetET, jetEta, jetPhi);

      jets.push_back(theJet);
    }
  }
  jets.sort();
  jets.reverse();
}
