#include "GeneratorInterface/Core/interface/GenFilterEfficiencyProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

GenFilterEfficiencyProducer::GenFilterEfficiencyProducer(const edm::ParameterSet& iConfig) :
  filterPath(iConfig.getParameter<std::string>("filterPath")),
  tns_(),
  thisProcess(),pathIndex(100000),
  numEventsPassPos_(0),
  numEventsPassNeg_(0),
  numEventsFailPos_(0),
  numEventsFailNeg_(0),
  sumpass_w_(0.),
  sumpass_w2_(0.),
  sumfail_w_(0.),
  sumfail_w2_(0.)
{
   //now do what ever initialization is needed
  if (edm::Service<edm::service::TriggerNamesService>().isAvailable()) {
    // get tns pointer
    tns_ = edm::Service<edm::service::TriggerNamesService>().operator->();
    if (tns_!=0) {
      thisProcess = tns_->getProcessName();
      std::vector<std::string> theNames = tns_->getTrigPaths();
      for ( unsigned int i = 0; i < theNames.size(); i++ ) {
        if ( theNames[i] == filterPath ) { pathIndex = i; continue; }
      }
    }
    else
      edm::LogError("ServiceNotAvailable") << "TriggerNamesServive not available, no filter information stored";
  }

  produces<GenFilterInfo, edm::InLumi>(); 

}


GenFilterEfficiencyProducer::~GenFilterEfficiencyProducer()
{

} 


//
// member functions
//

// ------------ method called to for each event  ------------
void
GenFilterEfficiencyProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  edm::InputTag theTrig("TriggerResults","",thisProcess);
  edm::Handle<edm::TriggerResults> trigR;
  iEvent.getByLabel(theTrig,trigR); 
  edm::Handle<GenEventInfoProduct>    genEventScale;
  double weight = 1;
  if (iEvent.getByLabel("generator", genEventScale)) 
    weight = genEventScale->weight();
  

  unsigned int nSize = (*trigR).size();
  // std::cout << "Number of paths in TriggerResults = " << nSize  << std::endl;
  if ( nSize >= pathIndex ) {

    if (!trigR->wasrun(pathIndex))return;
    if ( trigR->accept(pathIndex) ) { 
      sumpass_w_ += weight;
      sumpass_w2_+= weight*weight;
      if(weight > 0)
	numEventsPassPos_++;
      else
	numEventsPassNeg_++;

    }
    else // if fail the filter
      {
      sumfail_w_ += weight;
      sumfail_w2_+= weight*weight;
      if(weight > 0)
	numEventsFailPos_++;
      else
	numEventsFailNeg_++;
      }
    //    std::cout << "Total events = " << numEventsTotal << " passed = " << numEventsPassed << std::endl;

  }
  
}

void
GenFilterEfficiencyProducer::beginLuminosityBlock(edm::LuminosityBlock const&, const edm::EventSetup&) {

  numEventsPassPos_=0;
  numEventsPassNeg_=0;
  numEventsFailPos_=0;
  numEventsFailNeg_=0;
  sumpass_w_=0;
  sumpass_w2_=0;
  sumfail_w_=0;
  sumfail_w2_=0;
 
}
void
GenFilterEfficiencyProducer::endLuminosityBlock(edm::LuminosityBlock const& iLumi, const edm::EventSetup&) {
}

void
GenFilterEfficiencyProducer::endLuminosityBlockProduce(edm::LuminosityBlock & iLumi, const edm::EventSetup&) {

  std::auto_ptr<GenFilterInfo> thisProduct(new GenFilterInfo(
							     numEventsPassPos_,
							     numEventsPassNeg_,
							     numEventsFailPos_,
							     numEventsFailNeg_,
							     sumpass_w_,
							     sumpass_w2_,
							     sumfail_w_,
							     sumfail_w2_
							     ));
  iLumi.put(thisProduct);
}
