#!/usr/bin/env python3
"""
GitHub Trending Collector

This script collects trending repository data from GitHub and saves it to JSON/CSV files.
Supports different languages and time ranges (daily, weekly, monthly).
"""

import argparse
import json
import os
from datetime import datetime
from typing import List, Dict, Optional

import requests
from bs4 import BeautifulSoup


class GitHubTrendingCollector:
    """Collector for GitHub trending repositories."""
    
    BASE_URL = "https://github.com/trending"
    
    def __init__(self, language: Optional[str] = None, since: str = "daily"):
        """
        Initialize the collector.
        
        Args:
            language: Programming language filter (e.g., 'python', 'javascript', None for all)
            since: Time range ('daily', 'weekly', 'monthly')
        """
        self.language = language
        self.since = since
        self.session = requests.Session()
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
        })
    
    def get_trending_url(self) -> str:
        """Construct the trending URL based on language and time range."""
        url = self.BASE_URL
        if self.language:
            url += f"/{self.language}"
        url += f"?since={self.since}"
        return url
    
    def fetch_trending(self) -> List[Dict]:
        """
        Fetch trending repositories from GitHub.
        
        Returns:
            List of dictionaries containing repository information
        """
        url = self.get_trending_url()
        print(f"Fetching trending repositories from: {url}")
        
        try:
            response = self.session.get(url, timeout=30)
            response.raise_for_status()
        except requests.RequestException as e:
            print(f"Error fetching data: {e}")
            return []
        
        return self._parse_trending_page(response.text)
    
    def _parse_trending_page(self, html: str) -> List[Dict]:
        """
        Parse the trending page HTML to extract repository information.
        
        Args:
            html: HTML content of the trending page
            
        Returns:
            List of repository data dictionaries
        """
        soup = BeautifulSoup(html, 'lxml')
        repositories = []
        
        articles = soup.find_all('article', class_='Box-row')
        
        for article in articles:
            try:
                repo_data = self._extract_repo_data(article)
                if repo_data:
                    repositories.append(repo_data)
            except Exception as e:
                print(f"Error parsing repository: {e}")
                continue
        
        print(f"Successfully fetched {len(repositories)} repositories")
        return repositories
    
    def _extract_repo_data(self, article) -> Optional[Dict]:
        """
        Extract repository data from an article element.
        
        Args:
            article: BeautifulSoup article element
            
        Returns:
            Dictionary containing repository data
        """
        repo_data = {}
        
        title_elem = article.find('h2', class_='h3')
        if not title_elem:
            return None
        
        link_elem = title_elem.find('a')
        if not link_elem:
            return None
        
        repo_path = link_elem.get('href', '').strip()
        repo_data['url'] = f"https://github.com{repo_path}"
        repo_data['name'] = repo_path.strip('/')
        
        author_repo = repo_path.strip('/').split('/')
        if len(author_repo) == 2:
            repo_data['author'] = author_repo[0]
            repo_data['repository'] = author_repo[1]
        
        desc_elem = article.find('p', class_='col-9')
        repo_data['description'] = desc_elem.get_text(strip=True) if desc_elem else ''
        
        language_elem = article.find('span', itemprop='programmingLanguage')
        repo_data['language'] = language_elem.get_text(strip=True) if language_elem else ''
        
        stars_elem = article.find('svg', class_='octicon-star')
        if stars_elem:
            stars_parent = stars_elem.find_parent('a')
            if stars_parent:
                stars_text = stars_parent.get_text(strip=True)
                repo_data['stars'] = stars_text.replace(',', '')
        
        forks_elem = article.find('svg', class_='octicon-repo-forked')
        if forks_elem:
            forks_parent = forks_elem.find_parent('a')
            if forks_parent:
                forks_text = forks_parent.get_text(strip=True)
                repo_data['forks'] = forks_text.replace(',', '')
        
        stars_today_elem = article.find('span', class_='d-inline-block float-sm-right')
        if stars_today_elem:
            stars_today_text = stars_today_elem.get_text(strip=True)
            repo_data['stars_today'] = stars_today_text
        
        return repo_data
    
    def save_to_json(self, data: List[Dict], output_dir: str = "data") -> str:
        """
        Save collected data to a JSON file.
        
        Args:
            data: List of repository data
            output_dir: Directory to save the file
            
        Returns:
            Path to the saved file
        """
        os.makedirs(output_dir, exist_ok=True)
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        lang_suffix = f"_{self.language}" if self.language else "_all"
        filename = f"trending{lang_suffix}_{self.since}_{timestamp}.json"
        filepath = os.path.join(output_dir, filename)
        
        output_data = {
            'collected_at': datetime.now().isoformat(),
            'language': self.language or 'all',
            'since': self.since,
            'count': len(data),
            'repositories': data
        }
        
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(output_data, f, indent=2, ensure_ascii=False)
        
        print(f"Data saved to: {filepath}")
        return filepath
    
    def save_to_csv(self, data: List[Dict], output_dir: str = "data") -> str:
        """
        Save collected data to a CSV file.
        
        Args:
            data: List of repository data
            output_dir: Directory to save the file
            
        Returns:
            Path to the saved file
        """
        os.makedirs(output_dir, exist_ok=True)
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        lang_suffix = f"_{self.language}" if self.language else "_all"
        filename = f"trending{lang_suffix}_{self.since}_{timestamp}.csv"
        filepath = os.path.join(output_dir, filename)
        
        if not data:
            print("No data to save")
            return filepath
        
        keys = ['name', 'author', 'repository', 'description', 'language', 
                'stars', 'forks', 'stars_today', 'url']
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(','.join(keys) + '\n')
            
            for repo in data:
                values = []
                for key in keys:
                    value = str(repo.get(key, '')).replace(',', ';').replace('\n', ' ')
                    values.append(f'"{value}"')
                f.write(','.join(values) + '\n')
        
        print(f"Data saved to: {filepath}")
        return filepath


def main():
    """Main function to run the collector."""
    parser = argparse.ArgumentParser(
        description='Collect GitHub trending repository data'
    )
    parser.add_argument(
        '--language',
        type=str,
        default=None,
        help='Programming language filter (e.g., python, javascript, go)'
    )
    parser.add_argument(
        '--since',
        type=str,
        default='daily',
        choices=['daily', 'weekly', 'monthly'],
        help='Time range for trending repositories'
    )
    parser.add_argument(
        '--format',
        type=str,
        default='json',
        choices=['json', 'csv', 'both'],
        help='Output format'
    )
    parser.add_argument(
        '--output-dir',
        type=str,
        default='data',
        help='Output directory for saved files'
    )
    
    args = parser.parse_args()
    
    collector = GitHubTrendingCollector(language=args.language, since=args.since)
    trending_data = collector.fetch_trending()
    
    if not trending_data:
        print("No data collected")
        return
    
    if args.format in ['json', 'both']:
        collector.save_to_json(trending_data, args.output_dir)
    
    if args.format in ['csv', 'both']:
        collector.save_to_csv(trending_data, args.output_dir)


if __name__ == '__main__':
    main()
