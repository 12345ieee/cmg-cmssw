///
/// \class l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1
///
/// \author: Jim Brooke
///
/// Description: first iteration of stage 2 jet algo

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2EGammaAlgorithmFirmware.h"

#include "L1Trigger/L1TCalorimeter/interface/CaloStage2Nav.h"
#include "L1Trigger/L1TCalorimeter/interface/CaloTools.h"



l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::Stage2Layer2EGammaAlgorithmFirmwareImp1(CaloParams* params) :
  params_(params)
{

}


l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::~Stage2Layer2EGammaAlgorithmFirmwareImp1() {


}


void l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::processEvent(const std::vector<l1t::CaloCluster> & clusters,
							      const std::vector<l1t::CaloTower>& towers,
                    std::vector<l1t::EGamma> & egammas) {

  egammas.clear();
  for(size_t clusNr=0;clusNr<clusters.size();clusNr++){
    // Keep only actual clusters
    if(clusters[clusNr].isValid()){ 

      egammas.push_back(clusters[clusNr]);

      // Identification part 
      bool hOverEBit = idHOverE(clusters[clusNr]);
      bool fgBit = !(clusters[clusNr].hwSeedPt()>6 && clusters[clusNr].fgECAL()); 
      int qual = 0;
      if(fgBit) qual |= (0x1); // first bit = FG
      if(hOverEBit) qual |= (0x1<<1); // second bit = H/E
      egammas.back().setHwQual( qual ); 


      // Isolation part
      int hwEtSum = CaloTools::calHwEtSum(clusters[clusNr].hwEta(),clusters[clusNr].hwPhi(),towers,
          -1*params_->egIsoAreaNrTowersEta(),params_->egIsoAreaNrTowersEta(),
          -1*params_->egIsoAreaNrTowersPhi(),params_->egIsoAreaNrTowersPhi(),
          params_->egIsoMaxEtaAbsForIsoSum());
      int hwFootPrint = isoCalEgHwFootPrint(clusters[clusNr],towers);

      int nrTowers = CaloTools::calNrTowers(-1*params_->egIsoMaxEtaAbsForTowerSum(),
          params_->egIsoMaxEtaAbsForTowerSum(),
          1,72,towers,1,999,CaloTools::CALO);
      unsigned int lutAddress = isoLutIndex(egammas.back().hwEta(),nrTowers);

      int isolBit = hwEtSum-hwFootPrint <= params_->egIsolationLUT()->data(lutAddress); 
      // std::cout <<"hwEtSum "<<hwEtSum<<" hwFootPrint "<<hwFootPrint<<" isol "<<hwEtSum-hwFootPrint<<" bit "<<isolBit<<" area "<<params_->egIsoAreaNrTowersEta()<<" "<<params_->egIsoAreaNrTowersPhi()<< " veto "<<params_->egIsoVetoNrTowersPhi()<<std::endl;

      egammas.back().setHwIso(isolBit);
      //  egammas.back().setHwIso(hwEtSum-hwFootPrint); //naughtly little debug hack, shouldnt be in release, comment out if it is
    }//end of cuts on cluster to make EGamma
  }//end of cluster loop
}

bool l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::idHOverE(const l1t::CaloCluster& clus)
{
  unsigned int lutAddress = idHOverELutIndex(clus.hwEta()); 
  bool hOverEBit = ( clus.hOverE() <= params_->egMaxHOverELUT()->data(lutAddress) );
  hOverEBit |= ( clus.hwPt()>=floor(params_->egEtToRemoveHECut()/params_->egLsb()) );
  return hOverEBit;
}

unsigned int l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::idHOverELutIndex(int iEta)
{
  unsigned int iEtaNormed = abs(iEta);
  return iEtaNormed;
}

//calculates the footprint of the electron in hardware values
int l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::isoCalEgHwFootPrint(const l1t::CaloCluster& clus,const std::vector<l1t::CaloTower>& towers) 
{
  int iEta=clus.hwEta();
  int iPhi=clus.hwPhi();

  // hwEmEtSumLeft =  CaloTools::calHwEtSum(iEta,iPhi,towers,-1,-1,-1,1,CaloTools::ECAL);
  // int hwEmEtSumRight = CaloTools::calHwEtSum(iEta,iPhi,towers,1,1,-1,1,CaloTools::ECAL);
  
  int etaSide = clus.checkClusterFlag(CaloCluster::TRIM_LEFT) ? 1 : -1; //if we trimed left, its the right (ie +ve) side we want
  int phiSide = iEta>0 ? 1 : -1;
  
  int ecalHwFootPrint = CaloTools::calHwEtSum(iEta,iPhi,towers,0,0,
					      -1*params_->egIsoVetoNrTowersPhi(),params_->egIsoVetoNrTowersPhi(),
					      params_->egIsoMaxEtaAbsForIsoSum(),CaloTools::ECAL) +
    CaloTools::calHwEtSum(iEta,iPhi,towers,etaSide,etaSide,
			  -1*params_->egIsoVetoNrTowersPhi(),params_->egIsoVetoNrTowersPhi(),
			  params_->egIsoMaxEtaAbsForIsoSum(),CaloTools::ECAL);
  int hcalHwFootPrint = CaloTools::calHwEtSum(iEta,iPhi,towers,0,0,0,0,params_->egIsoMaxEtaAbsForIsoSum(),CaloTools::HCAL) +
    CaloTools::calHwEtSum(iEta,iPhi,towers,0,0,phiSide,phiSide,params_->egIsoMaxEtaAbsForIsoSum(),CaloTools::HCAL);
  return ecalHwFootPrint+hcalHwFootPrint;
}

//ieta =-28, nrTowers 0 is 0, increases to ieta28, nrTowers=kNrTowersInSum
unsigned l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1::isoLutIndex(int iEta,unsigned int nrTowers)
{
  const unsigned int kNrTowersInSum=72*params_->egIsoMaxEtaAbsForTowerSum()*2;
  const unsigned int kTowerGranularity=params_->egIsoPUEstTowerGranularity();
  const unsigned int kMaxAddress = kNrTowersInSum%kTowerGranularity==0 ? (kNrTowersInSum/kTowerGranularity+1)*28*2 : 
                                                                         (kNrTowersInSum/kTowerGranularity)*28*2;
  
  unsigned int nrTowersNormed = nrTowers/kTowerGranularity;
  
  unsigned int iEtaNormed = iEta+28;
  if(iEta>0) iEtaNormed--; //we skip zero
  
  if(std::abs(iEta)>28 || iEta==0 || nrTowers>kNrTowersInSum) return kMaxAddress;
  else return iEtaNormed*(kNrTowersInSum/kTowerGranularity+1)+nrTowersNormed;
  
}


 
