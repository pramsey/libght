Coding Style Guidelines
=======================

Much like the GLib/GTK+ guidelines and the GNU guidelines.

https://git.gnome.org/browse/gtk+/plain/docs/CODING-STYLE
http://www.gnu.org/prep/standards/standards.html#Writing-C

Indenting / Whitespace
----------------------

Use four spaces for tab, and ANSI style brackets, and use astyle to enforce/fix your code::

  astyle --style=ansi --indent=spaces=4 --convert-tabs

ANSI style brackets are like this::

  if ( something )
  {
     ght_function(param);
  }
  

Naming
------

* Type definitions, structs and enumerations should be named with "GhtTypeName" format, using a "Ght" prefix and camel casing other words.
* Function names should be named in glib/gtk style, using a "ght_" prefix and underscores to separate words: "ght_object_action".  Where the function is working against an object, name it and the action.
* Constants and #defines should use GHT_ALL_CAPS.

Commenting
----------

Comment functions using Doxygen style, so there's a chance we can generate docs automatically::

  /**
   * Does something funny
   *
   * @param p1       : first point
   * @param p2     : second point
   * @param q   : the quiet point
   *
   * @return a funny double number
   */
  int funny_function(const GhtPoint *p1, const GhtPoint *p2, GhtPoint *q){
          funny stuff here;
  }

Note that the comment does not bother to recapitulate the function name, Doxygen gives us that for free.

Function Signatures
-------------------

* Functions should always return an int error code (GhtErr) indicating success or failure. 
* Input parameters should be passed by reference as much as possible (for anything bigger than a standard primitive) using a const pointer.
* Output parameters should be passed by reference using a mutable pointer.

Memory
------

* Remember to allocate off the stack whenever you reasonably can


