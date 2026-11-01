# `avela_skin_piccolo`: Piccolo

The default official skin uses deep teal and Manrope. It targets the 320×170 landscape screen.

## Appearance and implementation

The player uses the lower content band for its spectrum, including peaks. Its deep teal palette and Manrope roles
are defined with the page layouts in this component.

The component's `include/avela_skin_piccolo/palette.hpp` defines its color roles. Its page skins and
`layouts/landscape_320x170` define the appearance of each screen; `assets/logo/avela_symbol.svg` supplies its logo.

Shared interfaces, optional-build guards, and the authoring procedure are defined in
[Writing a skin](../../../../docs/developer/SKINS.md). Follow the
[asset procedure](../../../../docs/developer/SKINS.md#assets) when regenerating its committed fonts and logo.
