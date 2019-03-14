# Historic Artefacts

Historic notes and writeups that are likely outdated or obsolete.

## Ancient type system description

Description of the BSE types and its type system around 1999, before the migration to GObject.

### BSE Structures

#### BseParam

A parameter that can be get/set on an object.
Each parameter has a parameter specification associated with it.

#### BseParamSpec\*

A parameter specification similar to an object's class or to
procedure classes, but for a given parameter type multiple
parameter specifications may exist (e.g. to implement integer
parameters with different ranges).
Thus the type system actually implements parameters as "unclassed" types.

#### BseObject

BseObject implements basic structure operations and provides the
necessary cruft for objects to have classes.

#### BseObjectClass

A class associated with each objects instance.
A class is referenced by its object instances.
introduces per-object parameters.

#### BseProcedure

A BseProcedure is a a special classed-only (i.e. not-object) type that
holds parameter specifications in it's calss structure and can be called.
Work is in progress to map certain procedures to certain object types, so
they can be thought of as special object-methods to do perform certain
object specific modifications.

#### BsePlugin

A plugin handle that holds information about existing plugins, such as
where the actuall plugin's implementation is stored on disk and what
types of objects are implemented by the plugin.

Plugin can provide:

- object and class implementations derived from BseEffect or BseSource
- procedure classes
- enum and flags definitions (currently lacks implementation)
- sample/song file loading and saving routines (currently lacks
  implementation)

#### BseType

type ids are provided for
objects & classes,
object interfaces,
parameters,
procedures,
enum & flags definitions.


### The BSE type system

BSE features a single inheritance object hierarchy with multiple interfaces.
It therefore contains a type system that registers several object and
non-object types. The type system features unclassed types (parameters),
classed types (enumerations and procedures) and object types (also classed).
The type system implemented by BSE has several analogies to the Gtk+ type
system and several type system related code portions have been traded between
the two projects. However, the BSE type system got furtherly enhanced (read:
complicated ;) in some regards by featuring dynamic types to allow certain
implementations to reside in dynamic plugins.

Usually, all types get registered with their name, description and inheritance
information upon startup and for static types the implementation related type
information is also stored right away. This information cannot be stored right
away for dynamic types since the implementation related type information
contains function pointers that will not remaind valid across the life time
of plugins (a plugin being loaded, unloaded and the reloaded again will most
certainly end up in a different memory location than before).
When class or object instances of a dynamic type need to be created, the
corresponding plugin will be loaded into memory and the required implementation
related type information will be retrieved.

Upon destruction of the instance, the type info is released again and the
plugin will be unloaded.
So for dynamic types, the only things that stays persistent across plugin life
times are the type's name, description, inheritance information and type id,
as well as additional information about what plugin this type is implemented
by.

Parameter types are unclassed and do not feature inheritance, they are merely
variable type ids to distinguish between e.g. strings and integers.
Enumeration types are classed, with flags being a certain kind of enumerations
in that their values only feature certain bits. For enumertaions, a flat
hierarchy is supported, where every enumeration or flags type derives from
`BSE_TYPE_ENUM` or `BSE_TYPE_FLAGS` respectively.

Object types are classed as well, and may exist in multiple instances per type
id. They feature single inheritance and multiple interfaces, where an interface
type denotes a certain alternate class-based vtable that is required to adress
similar object methods from different heirarchy branches with the same API.
Type inheritance can be examined by testing their `is_a` relationships, e.g.
for a type `parent` and a type `child` deriving from `parent`, the following
holds true: `child is_a parent`, while the reverse: `parent is_a child`
would fail, because `child` inherits all of `parent`'s facilities, but not
the other way around.

Whether a certain object type implements a specific interface, can be tested
through the `conforms_to` relationship, e.g. for an interface `I` requirering
a method implementation `foo` and a base type `B`, not implementing `foo` with
it's two child types `C` and `D`, both carrying different imlpementations of
`foo`, the following applies:

- `B conforms_to I`: `FALSE`
- `C conforms_to I`: `TRUE`
- `D conforms_to I`: `TRUE`

We will now outline the several steps that are taken by the type system to
create/destroy classes and objects. This is actually the guide line for the
type system's implementation, so feel free to skip this over ;)
In the below, unreferencing of certain objects may automatically cause their
destruction, info structure retrival and releasing may (for dynamic types)
accompany plugin loading/unloading.

#### initialization

- all static type ids + type infos get registered
- all dynamic type ids get registered
  (implies: all interface type ids are registered)
- static type infos are persistant

#### class creation

- type info is enforced
- parent class is referenced
- class gets allocated and initialized

#### object creation

- class is referenced
- object is allocated and initialized

#### object destruction

- object gets deallocated (destruction has already been handled by the object system)
- class gets unreferenced

#### class destruction

- class gets destructed and deallocated
- type info is released
- parent class is unreferenced

#### interface registration

- interface id gets embeded in the host type's interface entry list
- the interface is registered for all children as well

#### interface class creation

- class is given
- class is referenced
- interface info is enforced
- interface is referenced
- interface entry info is enforced
- interface class is allocated and initialized (and propagated to children)

#### interface class destruction

- interface class gets destructed and deallocated (and propagated to children)
- interface entry info is released
- interface gets unreferenced
- interface info is released
- class gets unreferenced
