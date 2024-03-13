#include "light.h"

void Light::ProvideInput(const Input& input) {
  // Presets: 0 - off, 1-9 = 2^key - 1
  std::vector<UINT> keys_codes = { DIKEYBOARD_0,
    DIKEYBOARD_1, DIKEYBOARD_2, DIKEYBOARD_3,
    DIKEYBOARD_4, DIKEYBOARD_5, DIKEYBOARD_6,
    DIKEYBOARD_7, DIKEYBOARD_8, DIKEYBOARD_9 };

  for (int i = 0; i < keys_codes.size(); i++)
    if (input.IsKeyPressed(keys_codes[i]))
      sp.SetWColor((float)(std::pow(2, i) - 1.f));
      
  // Smooth light changing
  auto status = input.IsPlusMinusPressed();
  auto newWval = sp.GetColor().w + status * 1.f;
  newWval = max(min(newWval, MAX_I), MIN_I);
  sp.SetWColor(newWval);

  if (input.IsKeyPressed(DIK_L))
  {
    showSource = !showSource;
  }
}
