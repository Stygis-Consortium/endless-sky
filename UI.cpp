/* UI.cpp
Michael Zahniser, 24 Feb 2014

Function definitions for the UI class.
*/

#include "UI.h"

#include "Panel.h"
#include "Screen.h"

#include <SDL/SDL.h>

using namespace std;



// Default constructor.
UI::UI()
	: isDone(false)
{
}



// Handle an event. The event is handed to each panel on the stack until one
// of them handles it. If none do, this returns false.
bool UI::Handle(const SDL_Event &event)
{
	bool handled = false;
	
	vector<shared_ptr<Panel>>::iterator it = stack.end();
	while(it != stack.begin() && !handled)
	{
		--it;
		if(event.type == SDL_MOUSEMOTION)
		{
			if(event.motion.state & SDL_BUTTON(1))
				handled = (*it)->Drag(event.motion.xrel, event.motion.yrel);
			else
				handled = (*it)->Hover(
					event.motion.x - Screen::Width() / 2,
					event.motion.y - Screen::Height() / 2);
		}
		else if(event.type == SDL_MOUSEBUTTONDOWN)
		{
			int x = event.button.x - Screen::Width() / 2;
			int y = event.button.y - Screen::Height() / 2;
			if(event.button.button == 1)
				handled = (*it)->Click(x, y);
			else if(event.button.button == 3)
				handled = (*it)->RClick(x, y);
		}
		else if(event.type == SDL_KEYDOWN)
			handled = (*it)->KeyDown(event.key.keysym.sym, event.key.keysym.mod);
		
		// If this panel does not want anything below it to receive events, do
		// not let this event trickle further down the stack.
		handled |= (*it)->TrapAllEvents();
	}
	
	return handled;
}



// Step all the panels forward (advance animations, move objects, etc.).
void UI::StepAll()
{
	// Handle any panels that should be added.
	for(shared_ptr<Panel> &panel : toPush)
		if(panel)
		{
			panel->SetUI(this);
			stack.push_back(panel);
		}
	toPush.clear();
	
	// These panels should be popped but not deleted (because someone else
	// owns them and is managing their creation and deletion).
	for(const Panel *panel : toPop)
	{
		for(auto it = stack.begin(); it != stack.end(); ++it)
			if(it->get() == panel)
			{
				it = stack.erase(it);
				break;
			}
	}
	toPop.clear();
	
	// Step all the panels.
	for(shared_ptr<Panel> &panel : stack)
		panel->Step(panel == stack.back());
}



// Draw all the panels.
void UI::DrawAll()
{
	// Find the topmost full-screen panel. Nothing below that needs to be drawn.
	vector<shared_ptr<Panel>>::const_iterator it = stack.end();
	while(it != stack.begin())
		if((*--it)->IsFullScreen())
			break;
	
	for( ; it != stack.end(); ++it)
		(*it)->Draw();
}



// Add the given panel to the stack. UI is responsible for deleting it.
void UI::Push(const std::shared_ptr<Panel> &panel)
{
	toPush.push_back(panel);
}



// Remove the given panel from the stack (if it is in it). The panel will be
// deleted at the start of the next time Step() is called, so it is safe for
// a panel to Pop() itself.
void UI::Pop(const Panel *panel)
{
	toPop.push_back(panel);
}



// Delete all the panels and clear the "done" flag.
void UI::Reset()
{
	stack.clear();
	toPush.clear();
	toPop.clear();
	isDone = false;
}



// Tell the UI to quit.
void UI::Quit()
{
	isDone = true;
}



// Check if it is time to quit.
bool UI::IsDone() const
{
	return isDone;
}



// Check if it is time to quit.
bool UI::IsEmpty() const
{
	return stack.empty() && toPush.empty();
}
