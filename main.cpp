#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    int document_count_ = 0;
    void DocumentCountId(const int& document_count){
	 document_count_ = document_count;
	 }
	
	void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
       for (const string& word:words){
	   word_to_documents_freqs_[word].insert({document_id,TfCount(word, words)});
		}
	}
	

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct Query {
    set<string> minus;
    set<string> plus;
};   
   
	map<string, map<int, double>> word_to_documents_freqs_;
    set<string> stop_words_;
	
	

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
	
	double TfCount(const string& word, const vector<string>& document){
    return count(document.begin(), document.end(),word)* (1./ document.size());
}

    
    
	Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if(word[0] == '-'){
            query_words.minus.insert(word.substr(1)); //проверяем что это минус слово и берем его без минуса
            }
            else {
                query_words.plus.insert(word); //плюс слова
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int,double> document_to_relevance; //id и релевантность
		vector<Document> id_relevance;
        vector<double> idf_query_words; //для хранния вычисления IDF всех слов в запросе
		
		/*проходим плюс слова и вычисляем IDF для каждого из них, сохраняя полученный 
		результат в вектор*/		
        for (const string& plus_word : query_words.plus){
			if(word_to_documents_freqs_.count(plus_word)){
              idf_query_words.push_back(log(static_cast <double>(document_count_ )/
                                      word_to_documents_freqs_.at(plus_word).size()));			
            }
        }    
        
        /* Вычисляем релевантность для каждого документа */
		for  (int id=0; id < document_count_; ++id){            
            int idf_count_position = 0;
			for (const string& plus_word : query_words.plus){
				if(word_to_documents_freqs_.count(plus_word)){
					if (word_to_documents_freqs_.at(plus_word).count(id)){
						document_to_relevance[id] += 
                            (word_to_documents_freqs_.at(plus_word).at(id)
													*idf_query_words.at(idf_count_position));
                    }                    
					++idf_count_position ;				
				
                }			
            }
        }          
            
        
		/* Убираем документы с минус словами*/		
		if (!query_words.minus.empty()){
			for (const string& minus_word : query_words.minus){
				if (word_to_documents_freqs_.count(minus_word)){
					for (const auto& [id, tf] : word_to_documents_freqs_.at(minus_word)){
						document_to_relevance.erase(id);								
					}
				}
			}
		}			
		
		
		
		for (const auto& [id, relevance] : document_to_relevance){
			id_relevance.push_back(Document(id,relevance));
		}
				
        idf_query_words.clear();
        return id_relevance;
    }   

  };


    
    SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
	search_server.document_count_ = document_count;
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}