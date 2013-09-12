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

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include <MyGUI.h>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>

#include <Controller/Controller.h>
#include <Core/ClockUtils.h>
#include <Core/Path.h>
#include <Base/ShowException.h>
#include <Core/Types.h>
#include <Core/GameHandle.h>
#include <View/View.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <CameraControl.h>
#include <MeshControl.h>


using namespace BFG;
using namespace boost::units;

struct LevelerModelState
{
	LevelerModelState(GameHandle handle, Event::Lane& lane) :
	mLane(lane)
	{
		mLane.connectV(A_QUIT, this, &LevelerModelState::shutDown);
		mLane.connectV(A_SCREENSHOT, this, &LevelerModelState::screenshot);
		mLane.connectLoop(this, &LevelerModelState::onLoop);
	}

	void shutDown()
	{
		mLane.emit(BFG::ID::VE_SHUTDOWN, Event::Void());
	}

	void screenshot()
	{
		mLane.emit(BFG::ID::VE_SCREENSHOT, Event::Void());
	}

	void onLoop(Event::TickData tickData)
	{
		tick(tickData.mTimeSinceLastTick);
	}
		
	void tick(const f32 timeSinceLastFrame)
	{
		if (timeSinceLastFrame < EPSILON_F)
			return;
	}

	Event::Lane& mLane;
};

struct LevelerViewState : public View::State
{
public:
	typedef std::vector<Tool::BaseFeature*> FeatureListT;

	LevelerViewState(GameHandle handle, Event::Lane& lane) :
	State(handle, lane),
	mControllerAdapter(handle, lane)
	{
		createGui();

		mData.reset(new SharedData);
		mData->mState = handle;
		mData->mCamera = generateHandle();

		Tool::BaseFeature* feature = new Tool::CameraControl(lane, mData);
		mLoadedFeatures.push_back(feature);
		feature->activate();

		mLoadedFeatures.push_back(new Tool::MeshControl(mData));

		mLane.connectV(A_UPDATE_FEATURES, this, &LevelerViewState::onUpdateFeatures);

		onUpdateFeatures();
	}

	~LevelerViewState()
	{
		mActiveFeatures.clear();

		FeatureListT::iterator it = mLoadedFeatures.begin();
		for (; it != mLoadedFeatures.end(); ++it)
		{
			(*it)->unload();
		}
		mLoadedFeatures.clear();

		if (!mContainer.empty())
		{
			MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
			layMan->unloadLayout(mContainer);
		}
	}

	void createGui()
	{
		using namespace MyGUI;

  		LayoutManager* layMan = LayoutManager::getInstancePtr();
		mContainer = layMan->loadLayout("Leveler.layout");
		if (mContainer.empty())
			throw std::runtime_error("Leveler.layout not found!");

		Widget* box = Gui::getInstance().findWidgetT("MenuBox");
		if (!box)
			throw std::runtime_error("MenuBox not found!");

		Widget* back = Gui::getInstance().findWidgetT("Backpanel");
		if (!back)
			throw std::runtime_error("Backpanel not found");

		IntSize boxSize = box->getSize();
		IntSize size = RenderManager::getInstance().getViewSize();
		// leave 1 pixel space to the sides
		box->setSize(size.width - 2, boxSize.height);  
		back->setSize(size);
	}

	void onUpdateFeatures()
	{
		mActiveFeatures.clear();

		FeatureListT::iterator it = mLoadedFeatures.begin();
		for (; it != mLoadedFeatures.end(); ++it)
		{
			bool active = (*it)->isActive();
			if (active)
			{
				mActiveFeatures.push_back(*it);
			}
		}
	}

	// Ogre loop
	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		FeatureListT::iterator it = mActiveFeatures.begin();
		for (; it != mActiveFeatures.end(); ++it)
		{
			(*it)->update(evt);
		}

		return true;
	}

private:
	virtual void pause(){};
	virtual void resume(){};

	BFG::View::ControllerMyGuiAdapter mControllerAdapter;

	FeatureListT mLoadedFeatures;
	FeatureListT mActiveFeatures;

	boost::shared_ptr<SharedData> mData;

	MyGUI::VectorWidgetPtr mContainer;
};

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	assert(loop);

	GameHandle levelerHandle = BFG::generateHandle();
	
	// Hack: Using leaking pointers, because vars would go out of scope
	LevelerModelState* lms = new LevelerModelState(levelerHandle, loop);
	LevelerViewState* lvs = new LevelerViewState(levelerHandle, loop);

	// Init Controller
	GameHandle handle = generateHandle();

	BFG::Controller_::ActionMapT actions;
	actions[A_QUIT] = "A_QUIT";
	actions[A_SCREENSHOT] = "A_SCREENSHOT";
	actions[A_UPDATE_FEATURES] = "A_UPDATE_FEATURES";

	BFG::Controller_::fillWithDefaultActions(actions);	
	BFG::Controller_::sendActionsToController(loop, actions);

	Path path;
	const std::string config_path = path.Expand("Leveler.xml");
	const std::string state_name = "Leveler";

	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);
	Controller_::StateInsertion si(config_path, state_name, handle, true, wa);

	EventFactory::Create<Controller_::ControlEvent>
	(
		loop,
		ID::CE_LOAD_STATE,
		si
	);

	loop->connect(A_QUIT, lms, &LevelerModelState::ControllerEventHandler);
	loop->connect(A_SCREENSHOT, lms, &LevelerModelState::ControllerEventHandler);
	loop->connect(A_UPDATE_FEATURES, lvs, &LevelerViewState::toolEventHandler);

	loop->registerLoopEventListener(lms, &LevelerModelState::LoopEventHandler);

	return 0;
}

int main( int argc, const char* argv[] ) try
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Leveler.log");

	EventLoop iLoop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Leveler: He levels everything!";

	BFG::Controller_::Main controllerMain(controllerFrequency);
	BFG::View::Main viewMain(caption);

	iLoop.addEntryPoint(viewMain.entryPoint());
	iLoop.addEntryPoint(controllerMain.entryPoint());
	iLoop.addEntryPoint(new Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
