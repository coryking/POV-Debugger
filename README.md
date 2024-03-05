This codebase is an attempt to fully understand how my hall effect sensor interacts with interrupts and things downstream of the interrupt service handler. I need to ensure every interrupt gets counted because for this POV display, timing is everything. Nailing the timing is how we can ensure the display stays aligned and doesn't start to drift around in circles.

Odds are very good this will become the full app once I figure out the basics.
