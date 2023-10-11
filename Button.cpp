/*
	Button - a small library for Arduino to handle button debouncing
	(modified)

	original: Michael "madleech" Adams
	modified: Uri "urish" Shaked, eugene-makeev, Mateusz Stadnik "matgla",
Petrovich "petrowhich"
	MIT licensed.
*/

#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin, uint8_t debounce_ms)
:  _pin(pin)
,  _delay(debounce_ms)
,  _state(HIGH)
,  _debounce_state(_state)
,  _ignore_until(0)
,  _has_changed(false)
,  _reported_repeats(0)
,  _repeat_delay_ms(-1)
,  _repeat_ms(-1)
{
}

void Button::begin()
{
	pinMode(_pin, INPUT_PULLUP);
}

//
// public methods
//

bool Button::read()
{
	// monitor pin changes during debounce time
	if (digitalRead(_pin) != _state)
	{
		_state = !_state;
		_ignore_until = millis() + _delay;
	}
	// if no changes in debounce time, the button state stable, update value
	else if (_ignore_until > millis())
	{
		_state = !_state;

		if (_state == RELEASED)
		{
			_reported_repeats = repeats_since_press();
		}

		else
		{
			_reported_repeats = 0;
		}

		_ignore_until = millis() + _delay;
		_has_changed = (_debounce_state != _state);
		_debounce_state = _state;
	}

	return _debounce_state;
}

// how many repeated press events occured since the button was pressed
uint16_t Button::repeat_count()
{
	return _state == PRESSED ? repeats_since_press() : _reported_repeats;
}

// has the button gone from off -> on or pressed repeatedly
bool Button::toggled()
{
	read();
	return has_changed();
}

// mostly internal, tells you if a button has changed after calling the read() function
bool Button::has_changed()
{
	if (_has_changed)
	{
		_has_changed = false;
		return true;
	}
	return false;
}

// has the button gone from off -> on
bool Button::pressed()
{
	return (read() == PRESSED && has_changed());
}

// has the button gone from on -> off
bool Button::released()
{
	if (read() == PRESSED)
	{
		uint16_t old_repeats = _reported_repeats;
		_reported_repeats = repeats_since_press();
		return (has_changed() || old_repeats != _reported_repeats);
	}
	else
	{
		return false;
	}
}

void Button::set_repeat(int16_t delay_ms, int16_t repeat_ms)
{
	_repeat_delay_ms = delay_ms > _delay ? delay_ms - _delay : 0;
	_repeat_ms = repeat_ms;
}

uint16_t Button::repeats_since_press()
{
 	if (_repeat_delay_ms == -1 || millis() < _ignore_until + _repeat_delay_ms)
	{
		return 0;
	}

	if (_repeat_ms <= 0)
	{
		return 1;
	}

	uint32_t press_time = millis() - _ignore_until;
	return 1 + (press_time - _repeat_delay_ms) / _repeat_ms;
}
