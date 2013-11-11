#ifndef MuonIsolation_ExtractorFromDeposits_H
#define MuonIsolation_ExtractorFromDeposits_H

#include "PhysicsTools/IsolationAlgos/interface/IsoDepositExtractor.h"

#include "DataFormats/RecoCandidate/interface/IsoDeposit.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositFwd.h"

#include <string>

namespace muonisolation {

class ExtractorFromDeposits : public reco::isodeposit::IsoDepositExtractor {

public:
  ExtractorFromDeposits(){};
  ExtractorFromDeposits(const edm::ParameterSet& par);

  virtual ~ExtractorFromDeposits(){}

  void registerProducts(edm::ConsumesCollector& iC);

  virtual void fillVetos ( const edm::Event & ev, const edm::EventSetup & evSetup, 
      const reco::TrackCollection & tracks);
  virtual reco::IsoDeposit deposit (const edm::Event & ev, const edm::EventSetup & evSetup, 
      const reco::Track & track) const;
  virtual reco::IsoDeposit deposit (const edm::Event & ev, const edm::EventSetup & evSetup, 
      const reco::TrackRef & track) const;

private:
  edm::InputTag theCollectionTag;
  edm::EDGetTokenT<reco::IsoDepositMap> isoToken_;
};

}

#endif

