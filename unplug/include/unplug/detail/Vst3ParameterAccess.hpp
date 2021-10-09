//------------------------------------------------------------------------
// Copyright(c) 2021 Dario Mambro.
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.
//------------------------------------------------------------------------

#pragma once
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "unplug/MidiMapping.hpp"
#include "unplug/StringConversion.hpp"
#include "unplug/detail/EditRegister.hpp"
#include "unplug/detail/ParameterFromUserInterfaceCoordinates.hpp"

namespace unplug::vst3 {

using EditControllerEx1 = Steinberg::Vst::EditControllerEx1;
using ParamValue = Steinberg::Vst::ParamValue;
using ParameterInfo = Steinberg::Vst::ParameterInfo;
using TChar = Steinberg::Vst::TChar;
using String128 = Steinberg::Vst::String128;
using tresult = Steinberg::tresult;
static constexpr auto kResultTrue = Steinberg::kResultTrue;
static constexpr auto kResultFalse = Steinberg::kResultFalse;

/**
 * The ParameterAccess class exposes the plugin parameters to the user interface.
 * */
class ParameterAccess final
{
public:
  /**
   * Get the plain value of the specified parameter
   * @index the index of the parameter
   * @return the value of the parameter
   * */
  double getValue(ParamIndex index);

  /**
   * Converts a plain value to a normalized value for the specified parameter
   * @index the index of the parameter
   * @value the plain value
   * @return the normalized value
   * */
  double normalizeValue(ParamIndex index, double value);

  /**
   * Converts a normalized value to a plain value for the specified parameter
   * @index the index of the parameter
   * @value the normalized value
   * @return the plain value
   * */
  double valueFromNormalized(ParamIndex index, double value);

  /**
   * Gets the normalized value of a parameter
   * @index the index of the parameter
   * @return the normalized value of the parameter
   * */
  double getValueNormalized(ParamIndex index);

  /**
   * Gets the plain default value of the specified parameter
   * @index the index of the parameter
   * @return the default value of the parameter
   * */
  double getDefaultValue(ParamIndex index);

  /**
   * Gets the normalized default value of the specified parameter
   * @index the index of the parameter
   * @return the normalized default value of the parameter
   * */
  double getDefaultValueNormalized(ParamIndex index);

  /**
   * Gets the minimum value of the specified parameter
   * @index the index of the parameter
   * @return the minimum value of the parameter
   * */
  double getMinValue(ParamIndex index);

  /**
   * Gets the maximum value of the specified parameter
   * @index the index of the parameter
   * @return the maximum value of the parameter
   * */
  double getMaxValue(ParamIndex index);

  /**
   * Sets the plain value of the specified parameter
   * @index the index of the parameter
   * @value the plain value to set the parameter to
   * @return true if the parameter index is valid, false otherwise
   * */
  bool setValue(ParamIndex index, double value);

  /**
   * Sets the normalized value of the specified parameter
   * @index the index of the parameter
   * @value the normalized value to set the parameter to
   * @return true if the parameter index is valid, false otherwise
   * */
  bool setValueNormalized(ParamIndex index, double value);

  /**
   * Marks the specified  as being edited by a specific control and tells the host about it (for the undo). It should be
   * called when the editing starts. The unplug widgets do this internally.
   * @index the index of the parameter
   * @control the name of the control that it is editing the parameter
   * @return true if the parameter index is valid and is not being edited, false otherwise
   * */
  bool beginEdit(ParamIndex index, std::string control);

  /**
   * Marks the specified parameter as not being edited and tells the host about it (for the undo).
   * The unplug widgets do this internally.
   * @index the index of the parameter
   * @return true if the parameter index is valid and it was being edited, false otherwise
   * */
  bool endEdit(ParamIndex index);

  /**
   * Tells if a parameter is currently being edited
   * @index the index of the parameter
   * @return true if the parameter index is valid, false otherwise
   * */
  bool isBeingEdited(ParamIndex index) const;

  /**
   * Gets the name of the control that it is editing the specified parameter
   * @index the index of the parameter
   * @return the name of the control that is editing the parameter
   * */
  std::string getEditingControl(ParamIndex index) const;

  /**
   * Converts a normalized value to a plain value in text form for the specified parameter.
   * @index the index of the parameter
   * @valueNormalized the normalized value to convert
   * @return a string holding the plain value in text
   * */
  std::string convertToText(ParamIndex index, double valueNormalized);

  /**
   * Gets the plain value in text form for the specified parameter.
   * @index the index of the parameter
   * @return a string holding the plain value of the parameter in text
   * */
  std::string getValueAsText(ParamIndex index);

  /**
   * Gets the name of the specified parameter.
   * @index the index of the parameter
   * @return the name of the parameter
   * */
  std::string getName(ParamIndex index);

  /**
   * Gets the measure unit of the specified parameter.
   * @index the index of the parameter
   * @return the measure unit of the parameter
   * */
  std::string getMeasureUnit(ParamIndex index);

  /**
   * Gets the number of steps of the specified parameter.
   * @index the index of the parameter
   * @return the number of steps of the parameter
   * */
  int getNumSteps(ParamIndex index);

  /**
   * Tells if the specified parameter is automatable.
   * @index the index of the parameter
   * @return true if the parameter is automatable, false otherwise
   * */
  bool canBeAutomated(ParamIndex index);

  /**
   * Tells if the specified parameter is a list parameter.
   * @index the index of the parameter
   * @return true if the parameter is a list, false otherwise
   * */
  bool isList(ParamIndex index);

  /**
   * Tells if the specified parameter is a the preset parameter.
   * @index the index of the parameter
   * @return true if the parameter is thhe preset parameter, false otherwise
   * */
  bool isProgramChange(ParamIndex index);

  /**
   * Tells if the specified parameter is a the bypass parameter.
   * @index the index of the parameter
   * @return true if the parameter is the bypass parameter, false otherwise
   * */
  bool isBypass(ParamIndex index);

  /**
   * Maps a midi control to a parameter, listening to a specific midi channels.
   * @index the index of the parameter
   * @midiControl the midi control to map to the parameter
   * @channel the midi channel to listen to
   * */
  void setMidiMapping(ParamIndex index, int midiControl, int channel);

  /**
   * Maps a midi control to a parameter, listening to all channels.
   * @index the index of the parameter
   * @midiControl the midi control to map to the parameter
   * */
  void setMidiMapping(ParamIndex index, int midiControl);

  /**
   * Relates a parameter to a rectangular section of the user interface.
   * Used internally by the widgets.
   * @index the index of the parameter
   * @left the left x coordinate of the rectangle
   * @top the top y coordinate of the rectangle
   * @right the right x coordinate of the rectangle
   * @top the top y coordinate of the rectangle
   * */
  void addParameterRectangle(ParamIndex index, int left, int top, int right, int bottom);

  /**
   * Clears all relationships between parameters and sections of the user interface.
   * */
  void clearParameterRectangles();

  /**
   * Find the parameter related to the a point in the user interface. Can be used by the host to know what parameter can
   * be edited when the mouse cursor is hovering the user interface. Used internally by the UnplugController.
   * */
  bool findParameterFromUserInterfaceCoordinates(int xPos, int yPos, ParamIndex& parameterTag);

  ParameterAccess(EditControllerEx1& controller, detail::MidiMapping& midiMapping);

  ~ParameterAccess();

#ifndef UNPLUG_EXPOSE_VST3STYLE_PARAMETER_API
private:
#endif

  bool getDefaultValue(ParamIndex index, double& result);
  bool getDefaultValueNormalized(ParamIndex index, double& result);
  bool getMinValue(ParamIndex index, double& result);
  bool getMaxValue(ParamIndex index, double& result);
  bool convertToText(ParamIndex index, double valueNormalized, std::string& result);
  bool convertFromText(ParamIndex index, double& value, std::string const& text);
  bool getMeasureUnit(ParamIndex index, std::string& result);
  bool canBeAutomated(ParamIndex index, bool& result);
  bool isList(ParamIndex index, bool& result);
  double convertFromText(ParamIndex index, std::string const& text);
  void setFromText(ParamIndex index, std::string const& text);
  bool getName(ParamIndex index, std::string& result);
  bool getNumSteps(ParamIndex index, int& result);
  bool isBypass(ParamIndex index, bool& result);
  bool isProgramChange(ParamIndex index, bool& result);

private:
  EditControllerEx1& controller;
  detail::MidiMapping& midiMapping;
  unplug::detail::ParameterEditRegister editRegister;
  unplug::detail::ParameterFromUserInterfaceCoordinates parameterFinder;
  inline static thread_local ParameterAccess* current = nullptr;
};

ParameterAccess& getParameters();

namespace detail {
void setParameters(ParameterAccess* parameterAccess);
} // namespace detail

} // namespace unplug::vst3