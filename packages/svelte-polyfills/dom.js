const isBeforeSupported =
  typeof Element !== "undefined" &&
  Element.prototype &&
  typeof Element.prototype.before === "function";

function addBeforePolyfill(proto) {

  proto.before = function before() {

    const parentNode = this.parentNode;

    if (!parentNode) {
      return;
    }

    const argumentList = Array.prototype.slice.call(arguments);
    const ownerDocument = this.ownerDocument || document;
    const fragment = ownerDocument.createDocumentFragment();

    for (const node of argumentList) {
      const nodeToInsert =
        typeof node === "string" ? ownerDocument.createTextNode(node) : node;
      fragment.appendChild(nodeToInsert);
    }

    parentNode.insertBefore(fragment, this);
  };
}

if (!isBeforeSupported) {

  const protoTargets = [];

  if (typeof Element !== "undefined" && Element.prototype) {
    protoTargets.push(Element.prototype);
  }

  if (typeof CharacterData !== "undefined" && CharacterData.prototype) {
    protoTargets.push(CharacterData.prototype);
  }

  if (typeof DocumentType !== "undefined" && DocumentType.prototype) {
    protoTargets.push(DocumentType.prototype);
  }

  for (const proto of protoTargets) {
    addBeforePolyfill(proto);
  }
}

// ============================================
// ParentNode methods: append, prepend
// ============================================

[Element.prototype, Document.prototype, DocumentFragment.prototype].forEach((item) => {

  // append
  if (!item.hasOwnProperty('append')) {

    Object.defineProperty(item, 'append', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function append(...args) {

        const docFrag = document.createDocumentFragment();
        args.forEach((argItem) => {
          docFrag.appendChild(argItem instanceof Node ? argItem : document.createTextNode(String(argItem)));
        });
        this.appendChild(docFrag);
      }
    });
  }

  // prepend
  if (!item.hasOwnProperty('prepend')) {

    Object.defineProperty(item, 'prepend', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function prepend(...args) {

        const docFrag = document.createDocumentFragment();
        args.forEach((argItem) => {
          docFrag.appendChild(argItem instanceof Node ? argItem : document.createTextNode(String(argItem)));
        });
        this.insertBefore(docFrag, this.firstChild);
      }
    });
  }
});

// ============================================
// ChildNode methods: after, remove, replaceWith
// ============================================

[Element.prototype, CharacterData.prototype, DocumentType.prototype].forEach((item) => {

  // after
  if (!item.hasOwnProperty('after')) {

    Object.defineProperty(item, 'after', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function after(...args) {

        const docFrag = document.createDocumentFragment();
        args.forEach((argItem) => {
          docFrag.appendChild(argItem instanceof Node ? argItem : document.createTextNode(String(argItem)));
        });

        if (this.parentNode) {
          this.parentNode.insertBefore(docFrag, this.nextSibling);
        }
      }
    });
  }

  // remove
  if (!item.hasOwnProperty('remove')) {

    Object.defineProperty(item, 'remove', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function remove() {
        if (this.parentNode) {
          this.parentNode.removeChild(this);
        }
      }
    });
  }

  // replaceWith
  if (!item.hasOwnProperty('replaceWith')) {

    Object.defineProperty(item, 'replaceWith', {
      configurable: true,
      enumerable: true,
      writable: true,
      value: function replaceWith(...args) {

        const docFrag = document.createDocumentFragment();
        args.forEach((argItem) => {
          docFrag.appendChild(argItem instanceof Node ? argItem : document.createTextNode(String(argItem)));
        });

        if (this.parentNode) {
          this.parentNode.replaceChild(docFrag, this);
        }
      }
    });
  }
});

// ============================================
// Element methods: closest, matches
// ============================================

if (!Element.prototype.matches) {

  Element.prototype.matches =
    Element.prototype.msMatchesSelector ||
    Element.prototype.webkitMatchesSelector ||
    function (s) {

      const matches = (this.document || this.ownerDocument).querySelectorAll(s);
      let i = matches.length;

      while (--i >= 0 && matches.item(i) !== this) {}

      return i > -1;
    };
}

if (!Element.prototype.closest) {

  Element.prototype.closest = function (s) {

    let el = this;
    do {
      if (el.matches(s)) return el;

      el = el.parentElement || el.parentNode;
    } while (el !== null && el.nodeType === 1);

    return null;
  };
}

// ============================================
// NodeList.forEach
// ============================================

if (typeof NodeList !== 'undefined' && NodeList.prototype && !NodeList.prototype.forEach) {
  NodeList.prototype.forEach = Array.prototype.forEach;
}

// ============================================
// CustomEvent constructor
// ============================================

if (typeof window !== 'undefined' && typeof window.CustomEvent !== 'function') {

  window.CustomEvent = function CustomEvent(event, params) {

    params = params || { bubbles: false, cancelable: false, detail: null };

    const evt = document.createEvent('CustomEvent');
    evt.initCustomEvent(event, params.bubbles, params.cancelable, params.detail);

    return evt;
  };
  window.CustomEvent.prototype = window.Event.prototype;
}

// ============================================
// classList.toggle with force parameter
// ============================================

if (typeof document !== 'undefined') {

  const testElement = document.createElement('_');
  testElement.classList.toggle('c', false);

  if (testElement.classList.contains('c')) {

    const _toggle = DOMTokenList.prototype.toggle;
    DOMTokenList.prototype.toggle = function (token, force) {

      if (arguments.length > 1 && this.contains(token) === !force) {
        return force;
      }

      return _toggle.call(this, token);
    };
  }
}
