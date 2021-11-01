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
#include "unplug/Index.hpp"
#include <array>

namespace unplug {

namespace MidiCC {
enum MidiCC
{
  BankSelect = 0,
  ModulationWheelCoarse,
  BreathControllerCoarse,
  FootPedalCoarse = 4,
  PortamentoTimeCoarse,
  DataEntryCoarse,
  VolumeCoarse,
  BalanceCoarse,
  PanpositionCoarse = 10,
  ExpressionCoarse,
  EffectControl1Coarse,
  EffectControl2Coarse,
  GeneralPurposeSlider1 = 16,
  GeneralPurposeSlider2,
  GeneralPurposeSlider3,
  GeneralPurposeSlider4,
  BankSelectFine = 32,
  ModulationWheelFine,
  BreathControllerFine,
  FootPedalFine = 36,
  PortamentoTimeFine,
  DataEntryFine,
  VolumeFine,
  BalanceFine,
  PanpositionFine = 42,
  ExpressionFine,
  EffectControl1Fine,
  EffectControl2Fine,
  HoldPedal = 64,
  Portamento,
  SustenutoPedal,
  SoftPedal,
  LegatoPedal,
  Hold2Pedal,
  SoundVariation,
  SoundTimbre,
  SoundReleaseTime,
  SoundAttackTime,
  SoundBrightness,
  SoundControl6,
  SoundControl7,
  SoundControl8,
  SoundControl9,
  SoundControl10,
  GeneralPurposeButton1,
  GeneralPurposeButton2,
  GeneralPurposeButton3,
  GeneralPurposeButton4,
  ReverbLevel = 91,
  TremoloLevel,
  ChorusLevel,
  CelesteLevel,
  PhaserLevel,
  DataButtonincrement,
  DataButtondecrement,
  NonRegisteredParameterFine,
  NoNRegisteredParameterCoarse,
  RegisteredParameterFine,
  RegisteredParameterCoarse,
  AllSoundOff = 120,
  AllControllersOff,
  LocalKeyboard,
  AllNotesOff,
  OmniModeOff,
  OmniModeOn,
  MonoOperation,
  PolyOperation,
  AfterTouch,
  PitchBend
};
}

namespace detail {
class MidiMappingSingleChannel final
{
public:
  static constexpr auto unmapped = ParamIndex{ std::numeric_limits<ParamIndex>::max() };

  MidiMappingSingleChannel();

  void mapParameter(ParamIndex paramIndex, int controller);

  ParamIndex getParameter(int controller) const;

private:
  std::array<ParamIndex, 130> midiMapping;
};

class MidiMapping final
{
public:
  static constexpr auto unmapped = MidiMappingSingleChannel::unmapped;

  void mapParameter(ParamIndex paramIndex, int controller, int channel);

  void mapParameter(ParamIndex paramIndex, int controller);

  ParamIndex getParameter(int controller, int channel) const;

private:
  std::array<MidiMappingSingleChannel, 16> midiMappingByChannel;
};

} // namespace detail
} // namespace unplug