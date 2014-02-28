/*
 *  Emily Behrendt
 *  eabehr, 1128821
 *  Thursday February 27, 2014
 *  CSE 333 Homework 3
 *
 *  Copyright 2011 Steven Gribble
 *
 *  This file is part of the UW CSE 333 course project sequence
 *  (333proj).
 *
 *  333proj is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  333proj is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with 333proj.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <algorithm>
#include <list>
#include "./QueryProcessor.h"

extern "C" {
  #include "./libhw1/CSE333.h"
}

namespace hw3 {

QueryProcessor::QueryProcessor(list<string> indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::iterator idx_iterator = indexlist_.begin();
  for (HWSize_t i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = new DocTableReader(fir.GetDocTableReader());
    itr_array_[i] = new IndexTableReader(fir.GetIndexTableReader());
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != NULL);
  Verify333(itr_array_ != NULL);
  for (HWSize_t i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);
  vector<QueryProcessor::QueryResult> finalresult;

  // MISSING:
  size_t numidx = arraylen_;
  size_t numwords = query.size();

  for (HWSize_t idx = 0; idx < numidx; idx++) {
    // get docidtable for first word
    DocIDTableReader *first_ditr = itr_array_[idx]->LookupWord(query[0]);
    if (first_ditr == NULL) {
      // if word 1 not in index, move on to next index
      continue;
    }
    // docid_element_header list for first word in query
    list<docid_element_header> initialdocs = first_ditr->GetDocIDList();

    for (size_t word = 1; word < numwords; word++) {
      // docidtablereader for next word
      DocIDTableReader *next_ditr = itr_array_[idx]->LookupWord(query[word]);
      if (next_ditr == NULL) {
        // word not found, move on to next index
        break;
      }
      // docid_element_header list for next word in query
      list<docid_element_header> tempdocs = next_ditr->GetDocIDList();

        // we have list for first word, list for next word. we want to delete
        // from our init list the docs that do not contain both words
        std::list<docid_element_header>::iterator iter;

        // for each doc in initial docs, see if contained in new list of docs
        for (iter = initialdocs.begin(); iter != initialdocs.end();) {
          // is the word contained in the temp list?
          bool isInTemp = false;

          for (docid_element_header temp : tempdocs) {
            if (temp.docid == iter->docid) {
              // docid is in both sets, so it contains both words
              isInTemp = true;
              // increment num occurences of words
              iter->num_positions += temp.num_positions;
              // can now move on to next word in initialdocs
              break;
            }
          }
          if (!isInTemp) {
            // doc id is not in both sets! must delete from initialdocs
            iter = initialdocs.erase(iter);
          } else {
            iter++;
          }
        }
      delete next_ditr;
    }
    delete first_ditr;

    // now we have looped through all the query words for this specific index
    // we must make a QueryResult for each docid left in initialdocs
    DocTableReader *currDTreader = dtr_array_[idx];

    for (docid_element_header elem : initialdocs) {
      // get doctablereader, find docnames from docid, and
      // create QueryResult object from that to add to finalresult list
      DocID_t d_id = elem.docid;
      HWSize_t rank = elem.num_positions;
      string docname;

      bool found = currDTreader->LookupDocID(d_id, &docname);
      if (found) {
        QueryProcessor::QueryResult res  = {docname, rank};
        finalresult.push_back(res);
      } else {
        continue;
      }
    }
  }

  // Sort the final results.
  std::sort(finalresult.begin(), finalresult.end());
  return finalresult;
}

}  // namespace hw3
