// Shared query contract typedefs for JS renderer and playback code.

/**
 * @typedef {Object} QueryRequest
 * @property {*} start Inclusive start of the queried window.
 * @property {*} end Exclusive end of the queried window.
 */

/**
 * @typedef {Object} QueryFragment
 * @property {*} wholeStart Full interval start where the value is active.
 * @property {*} wholeEnd Full interval end where the value is active.
 * @property {*} partStart Clipped interval start inside the current query window.
 * @property {*} partEnd Clipped interval end inside the current query window.
 * @property {string} value Rendered value for this interval.
 */

module.exports = {};