package phuby;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;

class Phuby extends RubyObject {

        public Phuby(Ruby runtime, RubyClass klazz) {
                super(runtime, klazz);
        }
}
