%.mk: %.in
	$(SILENTMSG) "\tSED\t$@\n"
	$(SILENTCMD) sed -e 's/^\([^( ]*\)(\(.*\))$$/$$(eval $$(call \1,\2))/g' $< >$@ || rm -f $@

%.o: %.cpp
	$(SILENTMSG) "\tCXX\t$@\n"
	$(SILENTCMD) $(CXX) $(CXXFLAGS) -c -o $@ $<
