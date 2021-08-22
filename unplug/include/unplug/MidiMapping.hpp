#pragma once
#include <array>

namespace unplug {

enum class MidiCC
{
  BankSelect = 0,
  ModulationWheelCoarse,
  BreathcontrollerCoarse,
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
  BreathcontrollerFine,
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

namespace detail {
class MidiMappingSingleChannel final
{
public:
  static constexpr auto unmapped = -1;

  MidiMappingSingleChannel() { std::fill(midiMapping.begin(), midiMapping.end(), unmapped); }

  void mapParameter(int parameterTag, MidiCC controller) { midiMapping[static_cast<int>(controller)] = parameterTag; }

  int getParameter(MidiCC controller) const { return midiMapping[static_cast<int>(controller)]; }

private:
  std::array<int, 130> midiMapping;
};
} // namespace detail

class MidiMapping final
{
public:
  static constexpr auto unmapped = detail::MidiMappingSingleChannel::unmapped;

  void mapParameter(int parameterTag, MidiCC controller, int channel)
  {
    assert(channel < midiMappingByChannel.size());
    if (channel < midiMappingByChannel.size()) {
      midiMappingByChannel[channel].mapParameter(parameterTag, controller);
    }
  }

  void mapParameter(int parameterTag, MidiCC controller)
  {
    for (auto& channelMidiMapping : midiMappingByChannel) {
      channelMidiMapping.mapParameter(parameterTag, controller);
    }
  }

  int getParameter(MidiCC controller, int channel) const
  {
    assert(channel < midiMappingByChannel.size());
    if (channel < midiMappingByChannel.size()) {
      return midiMappingByChannel[channel].getParameter(controller);
    }
    else
      return unmapped;
  }

private:
  std::array<detail::MidiMappingSingleChannel, 16> midiMappingByChannel;
};

} // namespace unplug