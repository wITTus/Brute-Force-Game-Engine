/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRIGGER_H__
#define TRIGGER_H__

#include <set>

#include <Base/Logger.h>
#include <Core/Math.h>

#include <Model/Managed.h>
#include <Model/Environment.h>

namespace BFG {

typedef std::vector<GameHandle> HandleVector;

namespace Trigger {

template <typename Unused = int>
class Sequence : public Managed,
                 boost::noncopyable
{
public:
	typedef Sequence<Unused> This;

	Sequence(Event::Lane& lane,
	         GameHandle handle,
	         const HandleVector& goHandles,
	         const Environment& environment,
	         f32 radius,
	         bool activated = true) :
	Managed(handle, "Trigger::Sequence", ID::OT_Trigger),
	mSubLane(lane.createSubLane()),
	mGoHandles(goHandles),
	mEnvironment(environment),
	mRadius(radius),
	mActivated(activated),
	mCurrentWP(0)
	{
		assert(! goHandles.empty() && "Trigger::Sequence has no handles!");
	
		mSubLane->connectV(ID::TE_CHECK_LOCATION, this, &This::onCheckLocation);
		mSubLane->connectV(ID::TE_RESET, this, &This::onReset);
	}
	                
	~Sequence()
	{
		mSubLane.reset();
	}
	
	void onCheckLocation()
	{
		if (! mActivated)
			return;
		assert(! "TODO: This is broken, adapt for new event here");
		//			onCheckLocation(te->getData().mGOhandle, boost::get<v3>(te->getData().mValue));
	}

private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
	{}

	virtual void internalSynchronize()
	{}

#if 0
	void onCheckLocation(GameHandle handle, const v3& position)
	{
		// Note: This can't be done in internalUpdate as we would be bothered
		//       to store every position of every object as member variable.
		//       I thought about adding a "mLatest" member var, but this can't
		//       be done, too, because internalUpdate is called once per tick
		//       whereas EventHandler might be called X-times per tick.

		GameHandle currentHandle = mGoHandles.at(mCurrentWP);

		const BFG::Location& go = mEnvironment.getGoValue<BFG::Location>(currentHandle, ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
		
		if (nearEnough(go.position, position, mRadius))
		{
			Fire(ID::TE_FIRED, handle, currentHandle);
			++mCurrentWP;
			
			if (mCurrentWP >= static_cast<s32>(mGoHandles.size()))
			{
				Fire(ID::TE_SEQUENCE_COMPLETE, handle, currentHandle);
				mActivated = false;
			}
		}
	}
#endif
	
	void onReset()
	{
		mCurrentWP = 0;
		mActivated = true;
	}
	
	void Fire(ID::ModelAction triggerEvent, GameHandle handle, GameHandle wpOwner) const
	{
		assert(! "TODO: Emit a new event here!");
#if 0
		mSubLane->emit
		(
			triggerEvent,
			handle,
			wpOwner,
			getHandle(),
			mCurrentWP
		);
#endif

		infolog << "Trigger fired ("
		        << ID::asStr(triggerEvent)
		        << ") by " << handle;
	}

	Event::SubLanePtr           mSubLane;
	HandleVector                mGoHandles;
	const Environment&          mEnvironment;
	f32                         mRadius;
	bool                        mActivated;
	s32                         mCurrentWP;
};

} // namespace Trigger
} // namespace BFG

#endif
