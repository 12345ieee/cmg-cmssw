#include "TrackingTools/PatternTools/interface/MeasurementEstimator.h"
#include "FWCore/Utilities/interface/Exception.h"

MeasurementEstimator::Local2DVector 
MeasurementEstimator::maximalLocalDisplacement( const TrajectoryStateOnSurface& ts,
						const BoundPlane& plane) const
{
   throw cms::Exception("TrackingTools/PatternTools","MeasurementEstimator: base class maximalLocalDisplacement called");
}
