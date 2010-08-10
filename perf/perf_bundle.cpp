#include <iostream>
#include <malloc.h>
#include <algorithm>
#include <string.h>
#include <stdint.h>

#include "perf_bundle.h"
#include "perf_event.h"
#include "perf.h"

class perf_bundle_event: public perf_event 
{
	virtual void handle_event(struct perf_event_header *header, void *cookie);
};


void perf_bundle_event::handle_event(struct perf_event_header *header, void *cookie)
{
	unsigned char *buffer;
	vector<void *> *vector;

	buffer = (unsigned char *)malloc(header->size);
	memcpy(buffer, header, header->size);	

	vector = (typeof(vector))cookie;
	vector->push_back(buffer);
}


void perf_bundle::add_event(const char *event_name)
{
	class perf_event *ev;

	ev = new class perf_bundle_event();

	ev->set_event_name(event_name);
	ev->set_cpu(5);

	events.push_back(ev);
}

void perf_bundle::start(void)
{
	unsigned int i;
	class perf_event *ev;

	for (i = 0; i < events.size(); i++) {
		ev = events[i];
		if (!ev)
			continue;
		ev->start();
	}		
}
void perf_bundle::stop(void)
{
	unsigned int i;
	class perf_event *ev;

	for (i = 0; i < events.size(); i++) {
		ev = events[i];
		if (!ev)
			continue;
		ev->stop();
	}		
}
void perf_bundle::clear(void)
{
	unsigned int i;
	class perf_event *ev;

	for (i = 0; i < events.size(); i++) {
		ev = events[i];
		if (!ev)
			continue;
		ev->clear();
	}		
}


struct trace_entry {
	__u32			size;
	unsigned short		type;
	unsigned char		flags;
	unsigned char		preempt_count;
	int			pid;
	int			tgid;
};


struct perf_sample {
	struct perf_event_header        header;
	struct trace_entry		trace;
};

static uint64_t timestamp(perf_event_header *event)
{
	struct perf_sample *sample;
	int i;
	unsigned char *x;

	if (event->type != PERF_RECORD_SAMPLE)
		return 0;

	sample = (struct perf_sample *)event;


	printf("header:\n");
	printf("	type  is %x \n", sample->header.type);
	printf("	misc  is %x \n", sample->header.misc);
	printf("	size  is %i \n", sample->header.size);
	printf("sample:\n");
	printf("	size  is %i / %x \n", sample->trace.size, sample->trace.size);
	printf("	type  is %i / %x \n", sample->trace.type, sample->trace.type);
	printf("	flags is %i / %x \n", sample->trace.flags, sample->trace.flags);
	printf("	p/c   is %i / %x \n", sample->trace.preempt_count, sample->trace.preempt_count);
	printf("	pid   is %i / %x \n", sample->trace.pid, sample->trace.pid);
	printf("	tgid  is %i / %x \n", sample->trace.tgid, sample->trace.tgid);

	x = (unsigned char *)sample;
	for (i = 0; i < sample->header.size; i++)
		printf("%02x ", *(x+i));
	printf("\n");

	return 0; //sample->trace.time;
	
}

static bool event_sort_function (void *i, void *j) 
{ 
	struct perf_event_header *I, *J;

	I = (struct perf_event_header *) i;
	J = (struct perf_event_header *) j;
	return (timestamp(I)<timestamp(J)); 
}

void perf_bundle::process(void)
{
	unsigned int i;
	class perf_event *ev;

	/* fixme: reserve enough space in the array in one go */
	for (i = 0; i < events.size(); i++) {
		ev = events[i];
		if (!ev)
			continue;
		ev->process(&records);
	}		
	sort(records.begin(), records.end(), event_sort_function);

	printf("We got %u records total \n", records.size());
	for (i = 0; i < records.size(); i++) {
		printf("Event %u has timestamp %llx \n", i, timestamp((struct perf_event_header*)records[i]));
	}
}
