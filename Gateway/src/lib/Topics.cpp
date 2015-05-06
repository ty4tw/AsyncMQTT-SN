/*
 * Topics.cpp
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  Created on: 2015/05/01
 *    Modified:
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 0.0.0
 */

#include "Topics.h"
#include "Messages.h"
#include <string>

using namespace tomyAsyncGateway;
extern uint16_t getUint16(uint8_t* pos);

/*=====================================
        Class Topic
 ======================================*/
Topic::Topic(){
    _topicId = 0;
    _topicType = 0;
    _callback = 0;
    _next = 0;
}


Topic::Topic(string topic){
	_topicId = 0;
	_topicName = topic;
}


Topic::~Topic(){

}

int Topic::hasWildCard(unsigned int* pos){
	unsigned int p = _topicName.find("+", 0);
	if( p != string::npos){
		*pos = p;
		return 1;
	}else{
		string::iterator it = _topicName.end();
		if (*it == '#'){
			*pos = _topicName.size() - 1;
			return 2;
		}
	}
	*pos = 0;
    return 0;
}

bool Topic::isMatch(string* topicName){
	unsigned int pos;

	if ( topicName->size() < _topicName.size()){
		return false;
	}

	if (hasWildCard(&pos) == 1){
		if (_topicName.compare(0, pos - 1, *topicName, 0, pos - 1) == 0){
			if (_topicName.compare(pos + 1, 1, "/") == 0){
				unsigned int loc = topicName->find('/', pos);
				if (loc != 0){
					if (_topicName.compare(pos+1, _topicName.size() - pos - 1, *topicName, loc, topicName->size() - pos - 1) == 0){
						return true;
					}
				}
			}else{
				unsigned int loc = _topicName.find(pos, '/');
				if (loc != 0){
					if (topicName->find('/',loc) != 0){
						return false;
					}
				}
			}
			return true;
		}
	}else if (hasWildCard(&pos) == 2 &&
    		(_topicName.compare(0, pos, *topicName, 0, pos) == 0 )){
			return true;
	}else if (_topicName.compare(*topicName) == 0){
		return true;
	}
	return false;
}

uint16_t Topic::getTopicId(void){
	return _topicId;
}

string* Topic::getTopicName(void){
	return &_topicName;
}

/*=====================================
        Class Topics
 ======================================*/
Topics::Topics(){
	_first = 0;
	_nextTopicId = 0;
}

Topics::~Topics() {
	//clearTopic();
}


uint16_t Topics::getTopicId(string* topic){
	Topic* p = _first;
	while(p){
		if (p->_topicName.compare(*topic) == 0){
			return p->_topicId;
		}
		p = p->_next;
	}
	return 0;
}


Topic* Topics::getTopic(string* topic) {
	Topic* p = _first;
	while(p){
		if (p->_topicName.compare(*topic) == 0){
			return p;
		}
		p = p->_next;
	}
	return 0;
}

Topic* Topics::getTopic(uint16_t id, uint8_t topicType) {
	Topic* p = _first;
	while(p){
		if (p->_topicId == id && p->_topicType == topicType){
			return p;
		}
		p = p->_next;
	}
	return 0;
}

Topic* Topics::add(string* topicName, uint16_t id, uint8_t type, TopicCallback callback){
	Topic* elm;
	if (topicName){
		elm = getTopic(topicName);
	}else{
		elm = getTopic(id, type);
	}

	if (elm == 0){
		Topic* tp = _first;
		elm = new Topic();
		if(elm == 0){
			return 0;
		}
		elm->_topicName = *topicName;
		if ( type == MQTTSN_TOPIC_TYPE_SHORT){
			elm->_topicId = 0;
		}else{
			elm->_topicId = getNextTopicId();
		}
		elm->_topicType = type;
		elm->_callback = callback;

		if (tp == 0){
			_first = elm;
		}
		while(tp){
			if(tp->_next == 0){
				tp->_next = elm;
				break;
			}else{
				tp = tp->_next;
			}
		}
	}else{
		elm->_callback = callback;
	}
	return elm;
}


uint16_t Topics::getNextTopicId(){
	if(++_nextTopicId <  MQTTSN_TOPICID_NORMAL){
		_nextTopicId = MQTTSN_TOPICID_NORMAL;
	}
	return _nextTopicId;
}

Topic* Topics::match(string* topic){
	Topic* elm = _first;
	while(elm){
		if (elm->isMatch(topic)){
			return elm;
		}
		elm = elm->_next;
	}
	return 0;
}
