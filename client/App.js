import React from "./es-react/dev/react.js";
import htm from "https://unpkg.com/htm@latest?module";
const html = htm.bind(React.createElement);

import Markup from "./interweave/esm/Markup.js";

const App = (props) => {
  return html`<${Markup} content=${props.foo} />`;
}

export { App as default };