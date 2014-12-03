#ifndef FastSimulation_Tracking_TrajectorySeedProducer2_h
#define FastSimulation_Tracking_TrajectorySeedProducer2_h

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "FastSimulation/Tracking/interface/TrajectorySeedHitCandidate.h"

#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "FastSimulation/Tracking/interface/TrajectorySeedHitCandidate.h"

#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSRecHit2DCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiTrackerGSMatchedRecHit2DCollection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"

#include "FastSimulation/Tracking/interface/SeedingTree.h"
#include "FastSimulation/Tracking/interface/TrackingLayer.h"

#include <vector>
#include <sstream>


class MagneticField;
class MagneticFieldMap;
class TrackerGeometry;
class PropagatorWithMaterial;

class TrajectorySeedProducer: 
    public edm::EDProducer
{
    private:
        SeedingTree<TrackingLayer> _seedingTree;


        const MagneticField* magneticField;
        const MagneticFieldMap* magneticFieldMap;
        const TrackerGeometry* trackerGeometry;
        const TrackerTopology* trackerTopology;
        
        PropagatorWithMaterial* thePropagator;

        double pTMin;
        double maxD0;
        double maxZ0;
        unsigned int minRecHits;
        edm::InputTag hitProducer;
        edm::InputTag theBeamSpot;

        bool seedCleaning;
        bool rejectOverlaps;
        unsigned int absMinRecHits;
        unsigned int numberOfHits;
        
        std::string outputSeedCollectionName;


        std::vector<std::vector<TrackingLayer>> seedingLayers;
        
        math::XYZPoint beamspotPosition;

        double originRadius;
        double originHalfLength;
        double originpTMin;

        double zVertexConstraint;

        bool selectMuons;
        
        bool skipPVCompatibility;

        const reco::VertexCollection* vertices;
 
        // tokens
        edm::EDGetTokenT<reco::BeamSpot> beamSpotToken;
        edm::EDGetTokenT<edm::SimTrackContainer> simTrackToken;
        edm::EDGetTokenT<edm::SimVertexContainer> simVertexToken;
        edm::EDGetTokenT<SiTrackerGSMatchedRecHit2DCollection> recHitToken;
        edm::EDGetTokenT<reco::VertexCollection> recoVertexToken;

    public:

    TrajectorySeedProducer(const edm::ParameterSet& conf);
    
    //TODO: clean thing really up!!!!
    virtual ~TrajectorySeedProducer()
    {
    }

    virtual void beginRun(edm::Run const& run, const edm::EventSetup & es);
    virtual void produce(edm::Event& e, const edm::EventSetup& es);

    //! method checks if a SimTrack fulfills the requirements of the current seeding algorithm iteration.
    /*!
    \param theSimTrack the SimTrack to be tested.
    \param theSimVertex the associated SimVertex of the SimTrack.
    \param trackingAlgorithmId id of the seeding algorithm iteration (e.g. "initial step", etc.).
    \return true if a track fulfills the requirements.
    */
    virtual bool passSimTrackQualityCuts(const SimTrack& theSimTrack, const SimVertex& theSimVertex, unsigned int trackingAlgorithmId) const;

    //! method checks if a TrajectorySeedHitCandidate fulfills the requirements of the current seeding algorithm iteration.
    /*!
    \param trackerRecHits list of all TrackerRecHits.
    \param previousHits list of indexes of hits which already got accepted before.
    \param currentHit the current hit which needs to pass the criteria in addition to those in \e previousHits.
    \param trackingAlgorithmId id of the seeding algorithm iteration (e.g. "initial step", etc.).
    \return true if a hit fulfills the requirements.
    */
    inline bool passHitTuplesCuts(
            const SeedingNode<TrackingLayer>& seedingNode,
            const std::vector<TrajectorySeedHitCandidate>& trackerRecHits,
            const std::vector<int>& hitIndicesInTree,
            const TrajectorySeedHitCandidate& currentTrackerHit,
            unsigned int trackingAlgorithmId
        ) const
    {
        switch (seedingNode.getDepth())
        {
            case 0:
            {
                return true;
                /* example for 1 hits
                const TrajectorySeedHitCandidate& hit1 = currentTrackerHit;
                return pass1HitsCuts(hit1,trackingAlgorithmId);
                */
            }

            case 1:
            {
                const SeedingNode<TrackingLayer>* parentNode = &seedingNode;
                parentNode = parentNode->getParent();
                const TrajectorySeedHitCandidate& hit1 = trackerRecHits[hitIndicesInTree[parentNode->getIndex()]];
                const TrajectorySeedHitCandidate& hit2 = currentTrackerHit;

                return pass2HitsCuts(hit1,hit2,trackingAlgorithmId);
            }
            case 2:
            {
                return true;
                /* example for 3 hits
                const SeedingNode<LayerSpec>* parentNode = &seedingNode;
                parentNode = parentNode->getParent();
                const TrajectorySeedHitCandidate& hit2 = trackerRecHits[hitIndicesInTree[parentNode->getIndex()]];
                parentNode = parentNode->getParent();
                const TrajectorySeedHitCandidate& hit1 = trackerRecHits[hitIndicesInTree[parentNode->getIndex()]];
                const TrajectorySeedHitCandidate& hit3 = currentTrackerHit;
                return pass3HitsCuts(hit1,hit2,hit3,trackingAlgorithmId);
                */
            }
        }
        return true;
    }

    bool pass2HitsCuts(const TrajectorySeedHitCandidate& hit1, const TrajectorySeedHitCandidate& hit2, unsigned int trackingAlgorithmId) const;


    virtual std::vector<unsigned int> iterateHits(
            unsigned int start,
            const std::vector<TrajectorySeedHitCandidate>& trackerRecHits,
            std::vector<int> hitIndicesInTree,
            bool processSkippedHits,
            unsigned int trackingAlgorithmId
        ) const;

    inline bool isHitOnLayer(const TrajectorySeedHitCandidate& trackerRecHit, const TrackingLayer& layer) const
    {
        return layer==trackerRecHit.getTrackingLayer();
    }

    /// Check that the seed is compatible with a track coming from within
    /// a cylinder of radius originRadius, with a decent pT.
    bool compatibleWithBeamAxis(
            const GlobalPoint& gpos1, 
            const GlobalPoint& gpos2,
            double error,
            bool forward,
            unsigned algo
    ) const;

    const SeedingNode<TrackingLayer>* insertHit(
            const std::vector<TrajectorySeedHitCandidate>& trackerRecHits,
            std::vector<int>& hitIndicesInTree,
            const SeedingNode<TrackingLayer>* node, 
            unsigned int trackerHit,
            const unsigned int trackingAlgorithmId
    ) const;

};

#endif
