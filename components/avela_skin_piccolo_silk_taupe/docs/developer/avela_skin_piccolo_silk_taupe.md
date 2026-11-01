# `avela_skin_piccolo_silk_taupe`: Silk Taupe

The second official skin uses mushroom taupe, espresso, and champagne gold, with gold highlights. It targets the
320×170 landscape screen.

## Appearance and implementation

Silk Taupe owns its bronze accent, `gold_reflection` and `volume_track` colors, gold-gradient logo sun, and
`draw_gold_glint` helper. The helper draws a diagonal highlight on badges and the play button. Changes to this skin
must be checked independently of the default skin.

The component's `include/avela_skin_piccolo_silk_taupe/palette.hpp` defines its color roles. Its page skins and
`layouts/landscape_320x170` define the appearance of each screen; `assets/logo/avela_symbol.svg` supplies its logo.

Shared interfaces, optional-build guards, and the authoring procedure are defined in
[Writing a skin](../../../../docs/developer/SKINS.md). Follow the
[asset procedure](../../../../docs/developer/SKINS.md#assets) when regenerating its committed fonts and logo.
