//
// Package:         RecoTracker/RoadSearchSeedFinder
// Class:           RoadSearchSeedFinder
// 
// Description:     Calls RoadSeachSeedFinderAlgorithm
//                  to find TrackingSeeds.
//
// Original Author: Oliver Gutsche, gutsche@fnal.gov
// Created:         Sat Jan 14 22:00:00 UTC 2006
//
// $Author: stevew $
// $Date: 2006/02/08 02:06:48 $
// $Revision: 1.3 $
//

#include <iostream>
#include <memory>
#include <string>

#include "RecoTracker/RoadSearchSeedFinder/interface/RoadSearchSeedFinder.h"

#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DMatchedLocalPosCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPosCollection.h"
#include "DataFormats/TrackingSeed/interface/TrackingSeedCollection.h"

#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"

RoadSearchSeedFinder::RoadSearchSeedFinder(edm::ParameterSet const& conf) : 
  roadSearchSeedFinderAlgorithm_(conf) ,
  conf_(conf)
{
  produces<TrackingSeedCollection>();

}


// Virtual destructor needed.
RoadSearchSeedFinder::~RoadSearchSeedFinder() { }  

// Functions that gets called by framework every event
void RoadSearchSeedFinder::produce(edm::Event& e, const edm::EventSetup& es)
{

  // retrieve producer name of input SiStripRecHit2DLocalPosCollection
  std::string recHitProducer = conf_.getParameter<std::string>("RecHitProducer");
  
  // get Inputs
  edm::Handle<SiStripRecHit2DMatchedLocalPosCollection> matchedrecHits;
  e.getByLabel(recHitProducer,"matchedRecHit" ,matchedrecHits);
  edm::Handle<SiStripRecHit2DLocalPosCollection> rphirecHits;
  e.getByLabel(recHitProducer,"rphiRecHit" ,rphirecHits);

  // create empty output collection
  std::auto_ptr<TrackingSeedCollection> output(new TrackingSeedCollection);
  
  // invoke the seed finding algorithm
  roadSearchSeedFinderAlgorithm_.run(matchedrecHits,rphirecHits,es,*output);
  
  // write output to file
  e.put(output);

}
