Numpty Physics
==============

Harness gravity with your crayon and set about creating blocks, ramps, levers,
pulleys and whatever else you fancy to get the little red thing to the little
yellow thing.

Numpty Physics is a drawing puzzle game in the spirit (and style?) of Crayon
Physics using the same excellent Box2D engine. Note though that I've not
actually played CP so the experience may be very different. Numpty Physics
includes a built-in editor so that you may build (and submit) your own levels.

Numpty Physics is released under the GPL.

 * [Home page](http://thp.io/2015/numptyphysics/)
 * [Project page](http://github.com/thp/numptyphysics/)


Gameplay Hints
--------------

Each stroke is like a rigid piece of wire with a mass proportional to its
length. A closed stroke is just a wire bent into a shape, it has no substance
apart from its perimeter.

The ends of a strokes can (and will) join onto other strokes when drawn near
enough to another stroke. These joints are pivots so you can use this to build
levers, pendulums and other mechanical wonders.

Jointed strokes don't collide with each other. Join both ends to make a rigid
structure.


Editing Hints
-------------

It's handy to pause the physics when making a new level though this is by no
means necessary. Sometimes it is handy to let the physics run for a little bit
just to let the items settle down.

From the editor palette you can choose the crayon colour and then additional
properties.

You should make sure that your level has at least one red item (player token)
and at least one yellow item (goal item).

Token strokes will only join to other token strokes. Goal strokes will only
join to goal strokes. Other strokes will happily join to any non-token non-goal
strokes.
