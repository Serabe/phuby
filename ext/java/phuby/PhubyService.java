package phuby;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.load.BasicLibraryService;
import org.jruby.runtime.builtin.IRubyObject;

public class PhubyService implements BasicLibraryService {

        public boolean basicLoad(Ruby ruby) {
               RubyModule phuby = ruby.defineModule("Phuby");
               RubyClass runtime = ruby.defineClassUnder("Runtime", ruby.getObject(),
                               PHUBY_RUNTIME_ALLOCATOR, phuby);
               runtime.defineAnnotatedMethods(Phuby.class);
               return true;
        }

        private static final ObjectAllocator PHUBY_RUNTIME_ALLOCATOR = new ObjectAllocator(){
                public IRubyObject allocate(Ruby ruby, RubyClass klazz) {
                        return new Phuby(ruby, klazz);
                }
        };
}
