// -*- C++ -*-
//
// Package:     HigPhotonJetHLTOfflineSource
// Class:       HigPhotonJetHLTOfflineSource
// 

//
// Author: Xin Shi <Xin.Shi@cern.ch> 
// Created: 2014.07.22 
//

// system include files
#include <memory>
#include <iostream>

// user include files
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/PFJet.h"


//  Define the interface
class HigPhotonJetHLTOfflineSource : public DQMEDAnalyzer {

public:

  explicit HigPhotonJetHLTOfflineSource(const edm::ParameterSet&);

private:

  // Analyzer Methods
  virtual void beginJob();
  virtual void dqmBeginRun(const edm::Run &, const edm::EventSetup &) override;
  virtual void bookHistograms(DQMStore::IBooker &,
			      edm::Run const &, edm::EventSetup const &) override;  
  virtual void analyze(const edm::Event &, const edm::EventSetup &) override;
  virtual void endRun(const edm::Run &, const edm::EventSetup &) override;
  virtual void endJob();

  // Input from Configuration File
  edm::ParameterSet pset_;
  std::string hltProcessName_;
  std::vector<std::string> hltPathsToCheck_;
  std::string dirname_; 
  bool verbose_; 

  // Member Variables
  HLTConfigProvider hltConfig_;

  // Triggers 
  edm::EDGetTokenT <edm::TriggerResults> triggerResultsToken_;
  // CaloJet 
  edm::EDGetTokenT<reco::CaloJetCollection> caloJetsToken_;
  // Vertex 
  edm::EDGetTokenT<reco::VertexCollection> pvToken_;
  // Photon
  edm::EDGetTokenT<reco::PhotonCollection> photonsToken_;
  // PFMET
  edm::EDGetTokenT<reco::PFMETCollection> pfMetToken_;
  // PFJET
  edm::EDGetTokenT<reco::PFJetCollection> pfJetsToken_;


  MonitorElement*  ncalojets_;
  MonitorElement*  nvertices_;
  MonitorElement*  nphotons_;
  MonitorElement*  photonpt_;
  MonitorElement*  photonrapidity_;
  MonitorElement*  pfmet_;
  MonitorElement*  npfjets_;
  
};



// Class Methods 

HigPhotonJetHLTOfflineSource::HigPhotonJetHLTOfflineSource(const edm::ParameterSet& pset) :
  pset_(pset)
{
  hltProcessName_ = pset.getParameter<std::string>("hltProcessName"); 
  hltPathsToCheck_ = pset.getParameter<std::vector<std::string>>("hltPathsToCheck"); 
  verbose_ = pset.getUntrackedParameter<bool>("verbose", false);
  triggerResultsToken_ = consumes <edm::TriggerResults> (pset.getParameter<edm::InputTag>("triggerResultsToken"));
  dirname_ = pset.getUntrackedParameter<std::string>("dirname", std::string("HLT/Higgs/PhotonJet/"));
  caloJetsToken_ = consumes<reco::CaloJetCollection> (pset.getParameter<edm::InputTag>("caloJetsToken"));
  pvToken_ = consumes<reco::VertexCollection> (pset.getParameter<edm::InputTag>("pvToken"));
  photonsToken_ = consumes<reco::PhotonCollection> (pset.getParameter<edm::InputTag>("photonsToken"));
  pfMetToken_ = consumes<reco::PFMETCollection> (pset.getParameter<edm::InputTag>("pfMetToken"));
  pfJetsToken_ = consumes<reco::PFJetCollection> (pset.getParameter<edm::InputTag>("pfJetsToken"));


}

void 
HigPhotonJetHLTOfflineSource::dqmBeginRun(const edm::Run & iRun, 
					  const edm::EventSetup & iSetup) 
{ // Initialize hltConfig
  bool changedConfig;
  if (!hltConfig_.init(iRun, iSetup, hltProcessName_, changedConfig)) {
    edm::LogError("HLTPhotonJetVal") <<
      "Initialization of HLTConfigProvider failed!!"; 
    return;
  }
}


void 
HigPhotonJetHLTOfflineSource::bookHistograms(DQMStore::IBooker & iBooker, 
					     edm::Run const & iRun,
					     edm::EventSetup const & iSetup)
{
  iBooker.setCurrentFolder(dirname_);
  ncalojets_ = iBooker.book1D("ncalojets", "Number of Calo Jets", 100, 0., 100.);
  nvertices_ = iBooker.book1D("nvertices", "Number of vertices", 100, 0, 100); 
  nphotons_ = iBooker.book1D("nphotons", "Number of photons", 100, 0, 100); 
  photonpt_ = iBooker.book1D("photonpt", "Photons pT", 100, 0, 100); 
  photonrapidity_ = iBooker.book1D("photonrapidity", "Photons rapidity;y_{#gamma}", 100, -5, 5); 
  pfmet_ = iBooker.book1D("pfmet", "PF MET", 100, 0, 100); 
  npfjets_ = iBooker.book1D("npfjets", "Number of PF Jets", 100, 0, 100); 
}


void
HigPhotonJetHLTOfflineSource::analyze(const edm::Event& iEvent, 
				      const edm::EventSetup& iSetup)
{
  
  // Throw out this event if it doesn't pass any of the mornitored trigger.
  bool triggered = false; 
  edm::Handle<edm::TriggerResults> triggerResults;
  iEvent.getByToken(triggerResultsToken_, triggerResults);
  
  if(!triggerResults.isValid()) 
    {
      edm::LogError("HLTMuonMatchAndPlot")<<"Missing triggerResults collection" << std::endl;
      return;
    }

  // Check how many HLT triggers are in triggerResults
  const edm::TriggerNames triggerNames = iEvent.triggerNames(*triggerResults);
  for (unsigned int itrig = 0; itrig < triggerResults->size(); itrig++){
    // Only consider the triggered case. 
    // if ((*triggerResults)[itrig].accept() != 1) continue; 
    std::string triggername = triggerNames.triggerName(itrig);
    for (size_t i = 0; i < hltPathsToCheck_.size(); i++) {
      if ( triggername.find(hltPathsToCheck_[i]) != std::string::npos) {
	triggered = true;
	break;
      }
      if (triggered ) break;
    }
  }

  if (!triggered) return; 

  // CaloJet
  edm::Handle<reco::CaloJetCollection> calojets;
  iEvent.getByToken(caloJetsToken_, calojets);
  if(!calojets.isValid()) return;
  if (verbose_)
    std::cout << "xshi:: N calojets : " << calojets->size() << std::endl;

  ncalojets_->Fill(calojets->size()); 

  // N Vertices 
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(pvToken_, vertices);
  if(!vertices.isValid()) return;  
  if (verbose_)
    std::cout << "xshi:: N vertices : " << vertices->size() << std::endl;

  nvertices_->Fill(vertices->size());

  // photons pT
  edm::Handle<reco::PhotonCollection> photons;
  iEvent.getByToken(photonsToken_, photons);
  if(!photons.isValid()) return;  
  if (verbose_)
    std::cout << "xshi:: N photons : " << photons->size() << std::endl;
  nphotons_->Fill(photons->size());
  for(reco::PhotonCollection::const_iterator phoIter=photons->begin();
      phoIter!=photons->end();++phoIter){
    photonpt_->Fill(phoIter->pt()); 
    photonrapidity_->Fill(phoIter->rapidity()); 
  }

  // PF MET
  edm::Handle<reco::PFMETCollection> pfmets;
  iEvent.getByToken(pfMetToken_, pfmets);
  if (!pfmets.isValid()) return;
  if (verbose_)
    std::cout << "xshi:: N pfmets: " << pfmets->size() << std::endl;
  const reco::PFMET pfmet = pfmets->front();
  if (verbose_)
    std::cout << "xshi:: PFMET: " << pfmet.et() << std::endl;
  pfmet_->Fill(pfmet.et()); 

  // PF Jet
  edm::Handle<reco::PFJetCollection> pfjets;
  iEvent.getByToken(pfJetsToken_, pfjets);
  if(!pfjets.isValid()) return;
  if (verbose_)
    std::cout << "xshi:: N pfjets : " << pfjets->size() << std::endl;

  npfjets_->Fill(pfjets->size()); 

}


void 
HigPhotonJetHLTOfflineSource::beginJob()
{

}



void 
HigPhotonJetHLTOfflineSource::endRun(const edm::Run & iRun, 
				     const edm::EventSetup& iSetup)
{

}


void 
HigPhotonJetHLTOfflineSource::endJob()
{
 
}



//define this as a plug-in
DEFINE_FWK_MODULE(HigPhotonJetHLTOfflineSource);
