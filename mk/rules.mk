%.mk: %.in
	$(SILENTMSG) "\tSED\t$@"
	$(SILENTCMD) sed -e 's/^\([^( ]*\)(\(.*\))$$/$$(eval $$(call \1,\2))/g' $< >$@ || rm -f $@

%.o: %.cpp
	$(SILENTMSG) "\tCXX\t$@"
	$(SILENTCMD) $(CXX) $(CXXFLAGS) -c -o $@ $<
